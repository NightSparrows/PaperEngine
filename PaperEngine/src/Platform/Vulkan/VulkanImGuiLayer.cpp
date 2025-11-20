
#include <fstream>

#include "VulkanImGuiLayer.h"

#include <PaperEngine/core/Application.h>
#include <PaperEngine/events/ApplicationEvent.h>
#include <PaperEngine/events/KeyEvent.h>
#include <PaperEngine/events/MouseEvent.h>

#include <vulkan/vulkan.h>

// 因為我的Engine Vulkan一定使用GLFW
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
// We gather version tests as define in order to easily see which features are version-dependent.
#define GLFW_VERSION_COMBINED           (GLFW_VERSION_MAJOR * 1000 + GLFW_VERSION_MINOR * 100 + GLFW_VERSION_REVISION)
#define GLFW_HAS_WINDOW_TOPMOST         (GLFW_VERSION_COMBINED >= 3200) // 3.2+ GLFW_FLOATING
#define GLFW_HAS_WINDOW_HOVERED         (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_HOVERED
#define GLFW_HAS_WINDOW_ALPHA           (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwSetWindowOpacity
#define GLFW_HAS_PER_MONITOR_DPI        (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorContentScale
#if defined(__EMSCRIPTEN__) || defined(__SWITCH__)                      // no Vulkan support in GLFW for Emscripten or homebrew Nintendo Switch
#define GLFW_HAS_VULKAN                 (0)
#else
#define GLFW_HAS_VULKAN                 (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwCreateWindowSurface
#endif
#define GLFW_HAS_FOCUS_WINDOW           (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwFocusWindow
#define GLFW_HAS_FOCUS_ON_SHOW          (GLFW_VERSION_COMBINED >= 3300) // 3.3+ GLFW_FOCUS_ON_SHOW
#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorWorkarea
#define GLFW_HAS_OSX_WINDOW_POS_FIX     (GLFW_VERSION_COMBINED >= 3301) // 3.3.1+ Fixed: Resizing window repositions it on MacOS #1553
#ifdef GLFW_RESIZE_NESW_CURSOR          // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2019-11-29 (cursors defines) // FIXME: Remove when GLFW 3.4 is released?
#define GLFW_HAS_NEW_CURSORS            (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_RESIZE_ALL_CURSOR, GLFW_RESIZE_NESW_CURSOR, GLFW_RESIZE_NWSE_CURSOR, GLFW_NOT_ALLOWED_CURSOR
#else
#define GLFW_HAS_NEW_CURSORS            (0)
#endif
#ifdef GLFW_MOUSE_PASSTHROUGH           // Let's be nice to people who pulled GLFW between 2019-04-16 (3.4 define) and 2020-07-17 (passthrough)
#define GLFW_HAS_MOUSE_PASSTHROUGH      (GLFW_VERSION_COMBINED >= 3400) // 3.4+ GLFW_MOUSE_PASSTHROUGH
#else
#define GLFW_HAS_MOUSE_PASSTHROUGH      (0)
#endif
#define GLFW_HAS_GAMEPAD_API            (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetGamepadState() new api
#define GLFW_HAS_GETKEYNAME             (GLFW_VERSION_COMBINED >= 3200) // 3.2+ glfwGetKeyName()
#define GLFW_HAS_GETERROR               (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetError()
#define GLFW_HAS_GETPLATFORM            (GLFW_VERSION_COMBINED >= 3400) // 3.4+ glfwGetPlatform()

ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode);



namespace PaperEngine {

	void VulkanImGuiLayer::onAttach()
	{
		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional)

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f; // Set window background to opaque
		}



		m_commandList = device->createCommandList();

		// loading shaders
		{
			std::ifstream vertShaderFile(
				"assets/shaders/imgui/shader.vert.spv",
				std::ios::binary | std::ios::ate);

			if (!vertShaderFile.is_open())
			{
				PE_CORE_ERROR("Failed to open ImGui vertex shader file.");
				return;
			}
			size_t fileSize = vertShaderFile.tellg();
			vertShaderFile.seekg(0, std::ios::beg);
			std::vector<uint8_t> vertexShaderData(fileSize);
			vertShaderFile.read(reinterpret_cast<char*>(vertexShaderData.data()), fileSize);
			vertShaderFile.close();

			nvrhi::ShaderDesc shaderDesc;
			shaderDesc
				.setDebugName("ImGuiVertexShader")
				.setShaderType(nvrhi::ShaderType::Vertex)
				.setEntryName("main_vs");
			m_vertexShader = device->createShader(
				shaderDesc,
				vertexShaderData.data(), vertexShaderData.size());

			std::ifstream fragShaderFile(
				"assets/shaders/imgui/shader.frag.spv",
				std::ios::binary | std::ios::ate);

			if (!fragShaderFile.is_open())
			{
				PE_CORE_ERROR("Failed to open ImGui vertex shader file.");
				return;
			}
			fileSize = fragShaderFile.tellg();
			fragShaderFile.seekg(0, std::ios::beg);
			std::vector<uint8_t> fragShaderData(fileSize);
			fragShaderFile.read(reinterpret_cast<char*>(fragShaderData.data()), fileSize);
			fragShaderFile.close();

			shaderDesc
				.setDebugName("ImGuiFragmentShader")
				.setEntryName("main_ps")
				.setShaderType(nvrhi::ShaderType::Pixel);
			m_fragmentShader = device->createShader(
				shaderDesc,
				fragShaderData.data(), fragShaderData.size());

			if (!m_vertexShader || !m_fragmentShader)
			{
				PE_CORE_ERROR("Failed to create ImGui shaders.");
				return;
			}
		}

		nvrhi::VertexAttributeDesc vertexAttribLayouts[] = {
			{"POSITION",	nvrhi::Format::RG32_FLOAT,		1, 0, offsetof(ImDrawVert, pos), sizeof(ImDrawVert), false },
			{ "TEXCOORD",	nvrhi::Format::RG32_FLOAT,		1, 0, offsetof(ImDrawVert, uv), sizeof(ImDrawVert), false },
			{ "COLOR",		nvrhi::Format::RGBA8_UNORM,		1, 0, offsetof(ImDrawVert, col), sizeof(ImDrawVert), false }
		};

		m_shaderAttribLayout = device->createInputLayout(
			vertexAttribLayouts,
			sizeof(vertexAttribLayouts) / sizeof(nvrhi::VertexAttributeDesc),
			m_vertexShader);

		// create PSO desc
		{
			nvrhi::BlendState blendState;
			blendState.targets[0]
				.setBlendEnable(true)
				.setSrcBlend(nvrhi::BlendFactor::SrcAlpha)
				.setDestBlend(nvrhi::BlendFactor::InvSrcAlpha)
				.setSrcBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha)
				.setDestBlendAlpha(nvrhi::BlendFactor::Zero);

			auto rasterState = nvrhi::RasterState()
				.setFillSolid()
				.setCullNone()
				.setScissorEnable(true)
				.setDepthClipEnable(true);

			auto depthStencilState = nvrhi::DepthStencilState()
				.disableDepthTest()
				.enableDepthWrite()
				.disableStencil()
				.setDepthFunc(nvrhi::ComparisonFunc::Always);

			nvrhi::RenderState renderState;
			renderState.blendState = blendState;
			renderState.rasterState = rasterState;
			renderState.depthStencilState = depthStencilState;

			nvrhi::BindingLayoutDesc layoutDesc;
			layoutDesc.setVisibility(nvrhi::ShaderType::All);

			// 他會有validation error但不會掛
			layoutDesc.bindings = {
				nvrhi::BindingLayoutItem::PushConstants(0, sizeof(float) * 2),
				nvrhi::BindingLayoutItem::Texture_SRV(0),
				nvrhi::BindingLayoutItem::Sampler(0)
			};
			m_bindingLayout = device->createBindingLayout(layoutDesc);

			m_basePSODesc.primType = nvrhi::PrimitiveType::TriangleList;
			m_basePSODesc.inputLayout = m_shaderAttribLayout;
			m_basePSODesc.VS = m_vertexShader;
			m_basePSODesc.PS = m_fragmentShader;
			m_basePSODesc.renderState = renderState;
			m_basePSODesc.bindingLayouts = { m_bindingLayout };
		}

		{
			const auto desc = nvrhi::SamplerDesc()
				.setAllAddressModes(nvrhi::SamplerAddressMode::Wrap)
				.setAllFilters(true);
			m_fontSampler = device->createSampler(desc);

			if (m_fontSampler == nullptr) {
				PE_CORE_ERROR("failed to create imgui font sampler");
				return;
			}
		}

		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Application::Get()->getWindow()->getNativeWindow()), false);


		PE_CORE_TRACE("VulkanImGuiLayer attached successfully.");
	}

	void VulkanImGuiLayer::onDetach()
	{
		Application::GetNVRHIDevice()->waitForIdle();
		ImGui_ImplGlfw_Shutdown();
		m_commandList = nullptr;
		m_vertexShader = nullptr;
		m_fragmentShader = nullptr;
		m_shaderAttribLayout = nullptr;

		m_fontTexture = nullptr;
		m_fontSampler = nullptr;
		
		vertexBuffer = nullptr;
		indexBuffer = nullptr;

		m_bindingLayout = nullptr;
		m_basePSODesc = {};
		pso = nullptr;
		bindingCache.clear();

		vtxBuffer.clear();
		idxBuffer.clear();

		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::onUpdate(Timestep deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime.toSeconds();
	}

	void VulkanImGuiLayer::onPreRender()
	{
		this->updateFontTexture();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(
			float(Application::Get()->getWindow()->getWidth()),
			float(Application::Get()->getWindow()->getHeight()));

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();
	}

	void VulkanImGuiLayer::onFinalRender(nvrhi::IFramebuffer* fb)
	{
		// 手動畫Imgui
		ImGui::Render();
		ImDrawData* drawData = ImGui::GetDrawData();
		const auto& io = ImGui::GetIO();

		auto main_cmd = Application::Get()->getGraphicsContext()->getMainCommandList();
		
		main_cmd->beginMarker("ImGui");

		if (!updateGeometry())
		{
			return;
		}

		/// 他預設為Present跟DepthWrite

		main_cmd->setTextureState(fb->getDesc().colorAttachments[0].texture, nvrhi::AllSubresources, nvrhi::ResourceStates::RenderTarget);

		main_cmd->commitBarriers();

		drawData->ScaleClipRects(io.DisplayFramebufferScale);

		float invDisplaySize[2] = {
			1.0f / io.DisplaySize.x,
			1.0f / io.DisplaySize.y
		};

		nvrhi::GraphicsState drawState;


		drawState.framebuffer = fb;
		PE_CORE_ASSERT(fb, "Framebuffer is null in ImGui render.");

		drawState.pipeline = getPSO(fb);

		drawState.viewport.viewports.push_back(nvrhi::Viewport(io.DisplaySize.x * io.DisplayFramebufferScale.x,
			io.DisplaySize.y * io.DisplayFramebufferScale.y));

		drawState.viewport.scissorRects.resize(1);

		nvrhi::VertexBufferBinding vertexBinding;
		vertexBinding.buffer = vertexBuffer;
		vertexBinding.slot = 0;
		vertexBinding.offset = 0;
		drawState.vertexBuffers.push_back(vertexBinding);

		drawState.indexBuffer.buffer = indexBuffer;
		drawState.indexBuffer.format = (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT : nvrhi::Format::R32_UINT);
		drawState.indexBuffer.offset = 0;

		int vtxOffset = 0;
		int idxOffset = 0;
		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[n];
			for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
				const ImDrawCmd* pCmd = &cmdList->CmdBuffer[i];

				if (pCmd->UserCallback)
				{
					pCmd->UserCallback(cmdList, pCmd);
				}
				else {
					drawState.bindings = { getBindingSet((nvrhi::ITexture*)pCmd->GetTexID()) };
					PE_CORE_ASSERT(drawState.bindings[0], "ImGui binding set is null.");

					drawState.viewport.scissorRects[0] = nvrhi::Rect(
						int(pCmd->ClipRect.x),
						int(pCmd->ClipRect.z),
						int(pCmd->ClipRect.y),
						int(pCmd->ClipRect.w));

					nvrhi::DrawArguments drawArgs;
					drawArgs.vertexCount = pCmd->ElemCount;
					drawArgs.startIndexLocation = idxOffset;
					drawArgs.startVertexLocation = vtxOffset;

					main_cmd->setGraphicsState(drawState);
					main_cmd->setPushConstants(invDisplaySize, sizeof(invDisplaySize));
					main_cmd->drawIndexed(drawArgs);
				}

				idxOffset += pCmd->ElemCount;
			}

			vtxOffset += cmdList->VtxBuffer.Size;
		}


		main_cmd->endMarker();
		
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

	}

	void VulkanImGuiLayer::onEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {

			ImGuiIO& io = ImGui::GetIO();
			//ImGui_ImplGlfw_KeyCallback;
			if (!e.IsRepeat()) {
				ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(e.get_key_code(), e.get_scancode());
				io.AddKeyEvent(imgui_key, true);
				io.SetKeyEventNativeData(imgui_key, e.get_key_code(), e.get_scancode()); // To support legacy indexing (<1.87 user code)

				io.AddKeyEvent(ImGuiKey_ModCtrl, (e.get_mods() & GLFW_MOD_CONTROL) != 0);
				io.AddKeyEvent(ImGuiKey_ModShift, (e.get_mods() & GLFW_MOD_SHIFT) != 0);
				io.AddKeyEvent(ImGuiKey_ModAlt, (e.get_mods() & GLFW_MOD_ALT) != 0);
				io.AddKeyEvent(ImGuiKey_ModSuper, (e.get_mods() & GLFW_MOD_SUPER) != 0);
			}
			if (io.WantCaptureKeyboard)
				return true;	// handle
			return false;
			});
		dispatcher.dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) {

			ImGuiIO& io = ImGui::GetIO();
			ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(e.get_key_code(), e.get_scancode());
			io.AddKeyEvent(imgui_key, false);
			io.SetKeyEventNativeData(imgui_key, e.get_key_code(), e.get_scancode()); // To support legacy indexing (<1.87 user code)

			io.AddKeyEvent(ImGuiKey_ModCtrl, (e.get_mods() & GLFW_MOD_CONTROL) != 0);
			io.AddKeyEvent(ImGuiKey_ModShift, (e.get_mods() & GLFW_MOD_SHIFT) != 0);
			io.AddKeyEvent(ImGuiKey_ModAlt, (e.get_mods() & GLFW_MOD_ALT) != 0);
			io.AddKeyEvent(ImGuiKey_ModSuper, (e.get_mods() & GLFW_MOD_SUPER) != 0);
			if (io.WantCaptureKeyboard)
				return true;	// handle
			return false;
			});
		dispatcher.dispatch<KeyTypedEvent>([](KeyTypedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter(e.get_key_code());
			if (io.WantCaptureKeyboard)
				return true;	// handle
			return false;
			});
		dispatcher.dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) {

			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(e.GetMouseButton(), true);
			if (io.WantCaptureMouse)
				return true;
			return false;
			});
		dispatcher.dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(e.GetMouseButton(), false);
			if (io.WantCaptureMouse)
				return true;
			return false;
			});
		dispatcher.dispatch<MouseMovedEvent>([](MouseMovedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();

			float x = e.GetX(), y = e.GetY();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
				int window_x, window_y;
				glfwGetWindowPos((GLFWwindow*)Application::Get()->getWindow()->getNativeWindow(), &window_x, &window_y);
				x += window_x;
				y += window_y;
			}

			io.AddMousePosEvent(x, y);
			if (io.WantCaptureMouse)
				return true;
			return false;
			});
		dispatcher.dispatch<MouseScrolledEvent>([](MouseScrolledEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseWheelEvent(e.GetXOffset(), e.GetYOffset());
			if (io.WantCaptureMouse)
				return true;
			return false;
			});

		//dispatcher.dispatch<DisplayScaleChangedEvent>([](DisplayScaleChangedEvent& e) {
		//	ImGuiIO& io = ImGui::GetIO();

		//	io.Fonts->Clear();
		//	io.Fonts->SetTexID(0);

		//	ImGui::GetStyle() = ImGuiStyle();
		//	ImGui::GetStyle().ScaleAllSizes(e.getScaleX());

		//	return false;
		//	});
	}

	void VulkanImGuiLayer::onImGuiRender()
	{
	}

	void VulkanImGuiLayer::onBackBufferResizing()
	{
		pso = nullptr;
	}

	void VulkanImGuiLayer::onBackBufferResized()
	{
	}

	bool VulkanImGuiLayer::updateFontTexture()
	{
		ImGuiIO& io = ImGui::GetIO();

		if (m_fontTexture && io.Fonts->TexID.GetTexID())
			return true;

		unsigned char* pixels;
		int width, height;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		if (!pixels)
			return false;

		nvrhi::TextureDesc textureDesc;
		textureDesc
			.setWidth(width)
			.setHeight(height)
			.setFormat(nvrhi::Format::SRGBA8_UNORM)
			.setDebugName("ImGuiFontTexture");
		m_fontTexture = Application::Get()->getGraphicsContext()->getNVRhiDevice()->createTexture(textureDesc);

		if (!m_fontTexture)
			return false;

		m_commandList->open();

		m_commandList->beginTrackingTextureState(m_fontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);

		m_commandList->writeTexture(m_fontTexture, 0, 0, pixels, width * 4);
		
		m_commandList->setPermanentTextureState(
			m_fontTexture,
			nvrhi::ResourceStates::ShaderResource);
		m_commandList->commitBarriers();

		m_commandList->close();

		Application::Get()->getGraphicsContext()->getNVRhiDevice()->executeCommandList(m_commandList);

		io.Fonts->SetTexID(ImTextureRef(m_fontTexture));

		PE_CORE_TRACE("ImGui font texture updated successfully. view: {}", (void*)m_fontTexture);

		return true;
	}

	bool VulkanImGuiLayer::reallocateBuffer(nvrhi::BufferHandle& buffer, size_t requiredSize, size_t reallocateSize, const bool indexBuffer)
	{
		if (buffer == nullptr || size_t(buffer->getDesc().byteSize) < requiredSize)
		{
			nvrhi::BufferDesc desc;
			desc.byteSize = uint32_t(reallocateSize);
			desc.structStride = 0;
			desc.debugName = indexBuffer ? "ImGui index buffer" : "ImGui vertex buffer";
			desc.canHaveUAVs = false;
			desc.isVertexBuffer = !indexBuffer;
			desc.isIndexBuffer = indexBuffer;
			desc.isDrawIndirectArgs = false;
			desc.isVolatile = false;
			desc.initialState = indexBuffer ? nvrhi::ResourceStates::IndexBuffer : nvrhi::ResourceStates::VertexBuffer;
			desc.keepInitialState = true;

			buffer = Application::Get()->getGraphicsContext()->getNVRhiDevice()->createBuffer(desc);

			if (!buffer)
			{
				return false;
			}
		}

		return true;
	}

	bool VulkanImGuiLayer::updateGeometry()
	{
		ImDrawData* drawData = ImGui::GetDrawData();

		if (!reallocateBuffer(vertexBuffer, drawData->TotalVtxCount * sizeof(ImDrawVert),
			(drawData->TotalVtxCount + 5000) * sizeof(ImDrawVert),
			false))
		{
			return false;
		}

		if (!reallocateBuffer(indexBuffer, drawData->TotalIdxCount * sizeof(ImDrawIdx),
			(drawData->TotalIdxCount + 5000) * sizeof(ImDrawIdx),
			true))
		{
			return false;
		}

		vtxBuffer.resize(vertexBuffer->getDesc().byteSize / sizeof(ImDrawVert));
		idxBuffer.resize(indexBuffer->getDesc().byteSize / sizeof(ImDrawIdx));

		ImDrawVert* vtxDst = vtxBuffer.data();
		ImDrawIdx* idxDst = idxBuffer.data();

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[n];

			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		auto main_cmd = Application::Get()->getGraphicsContext()->getMainCommandList();

		main_cmd->writeBuffer(vertexBuffer, vtxBuffer.data(), vertexBuffer->getDesc().byteSize);
		main_cmd->writeBuffer(indexBuffer, idxBuffer.data(), indexBuffer->getDesc().byteSize);

		return true;
	}

	nvrhi::IGraphicsPipeline* VulkanImGuiLayer::getPSO(nvrhi::IFramebuffer* fb)
	{
		if (pso)
			return pso;

		pso = Application::Get()->getGraphicsContext()->getNVRhiDevice()->createGraphicsPipeline(
			m_basePSODesc,
			fb);
		PE_CORE_ASSERT(pso, "Failed to create ImGui PSO.");

		return pso;
	}

	nvrhi::IBindingSet* VulkanImGuiLayer::getBindingSet(nvrhi::ITexture* texture)
	{
		auto iter = bindingCache.find(texture);
		if (iter != bindingCache.end())
		{
			return iter->second;
		}

		nvrhi::BindingSetDesc desc;

		desc.bindings = {
			nvrhi::BindingSetItem::PushConstants(0, sizeof(float) * 2),
			nvrhi::BindingSetItem::Texture_SRV(0, texture),
			nvrhi::BindingSetItem::Sampler(0, m_fontSampler)
		};

		nvrhi::BindingSetHandle binding;
		binding = Application::Get()->getGraphicsContext()->getNVRhiDevice()->createBindingSet(
			desc,
			m_bindingLayout);
		PE_CORE_ASSERT(binding, "Failed to create ImGui binding set.");

		bindingCache[texture] = binding;
		return binding;
	}

}
