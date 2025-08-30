

#include "PaperEngine/utils/File.h"
#include <PaperEngine/core/Application.h>

#include "ForwardPlusDepthRenderer.h"

#include <nvrhi/utils.h>

namespace PaperEngine {

	void ForwardPlusDepthRenderer::init() {

		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		nvrhi::BufferDesc instanceBufferDesc;
		instanceBufferDesc
			.setByteSize(sizeof(glm::mat4) * 10000)
			.setIsConstantBuffer(true)
			.setKeepInitialState(true)
			.setStructStride(sizeof(glm::mat4))
			.setCpuAccess(nvrhi::CpuAccessMode::Write);
		m_instanceBuffer = Application::GetNVRHIDevice()->createBuffer(instanceBufferDesc);
		m_instanceBufferCpuPtr = Application::GetNVRHIDevice()->mapBuffer(m_instanceBuffer, nvrhi::CpuAccessMode::Write);

#pragma region Instance Buffer Initialization

		nvrhi::BindingLayoutDesc instanceBufLayoutDesc;
		instanceBufLayoutDesc
			.setRegisterSpace(1)			// set = 1
			.setRegisterSpaceIsDescriptorSet(true)
			.setVisibility(nvrhi::ShaderType::Vertex)
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0));
		m_instanceBufBindingLayout = CreateRef<BindingLayout>(Application::GetNVRHIDevice()->createBindingLayout(instanceBufLayoutDesc));

		nvrhi::BindingSetDesc instanceBufSetDesc;
		instanceBufSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_instanceBuffer));
		m_instanceBufferSet = Application::GetNVRHIDevice()->createBindingSet(instanceBufSetDesc, m_instanceBufBindingLayout->handle);

#pragma endregion

#pragma region Depth pass graphics Pipeline
		nvrhi::GraphicsPipelineDesc graphicsPipelineDesc;
		graphicsPipelineDesc
			.setPrimType(nvrhi::PrimitiveType::TriangleList)
			.renderState.rasterState.cullMode = nvrhi::RasterCullMode::Back;
		graphicsPipelineDesc.bindingLayouts.resize(2);
		graphicsPipelineDesc.bindingLayouts[0] = Application::GetResourceManager()
			->load<PaperEngine::BindingLayout>("SceneRenderer_globalLayout")->handle;
		graphicsPipelineDesc.bindingLayouts[1] = m_instanceBufBindingLayout->handle;
		{
			// vertex shader
			nvrhi::ShaderDesc shaderDesc;
			shaderDesc.debugName = "Test Vertex Shader";
			shaderDesc.entryName = "main_vs";
			shaderDesc.shaderType = nvrhi::ShaderType::Vertex;
			File file("assets/PaperEngine/shader/preDepthPass/shader.vert.spv");

			auto shaderBinary = file.readBinaryFully();
			graphicsPipelineDesc.VS = Application::GetNVRHIDevice()->createShader(
				shaderDesc,
				shaderBinary->data,
				shaderBinary->size);
		}

		nvrhi::VertexAttributeDesc attributes[] = {
			nvrhi::VertexAttributeDesc()
			.setName("POSITION")
			.setFormat(nvrhi::Format::RGB32_FLOAT)
			.setOffset(offsetof(PaperEngine::StaticVertex, position))
			.setBufferIndex(0)
			.setElementStride(sizeof(PaperEngine::StaticVertex))
		};

		graphicsPipelineDesc.inputLayout = PaperEngine::Application::GetNVRHIDevice()->createInputLayout(
			attributes,
			uint32_t(std::size(attributes)),
			graphicsPipelineDesc.VS
		);

		m_graphicsPipeline = PaperEngine::CreateRef<PaperEngine::GraphicsPipeline>(graphicsPipelineDesc, nullptr, 0);
#pragma endregion

	}
	ForwardPlusDepthRenderer::ForwardPlusDepthRenderer()
	{
	}

	ForwardPlusDepthRenderer::~ForwardPlusDepthRenderer()
	{
		Application::GetNVRHIDevice()->unmapBuffer(m_instanceBuffer);
		m_instanceBufferCpuPtr = nullptr;
	}

	void ForwardPlusDepthRenderer::addEntity(Ref<Mesh> mesh, const Transform& transform)
	{
		auto& instanceData = m_renderData[mesh].instanceData;
		instanceData.emplace_back(transform);
	}

	void ForwardPlusDepthRenderer::renderScene(const GlobalSceneData& globalData)
	{
		if (!m_framebuffer.handle) {
			onViewportResized(globalData.camera->getWidth(), globalData.camera->getHeight());
			return;
		}

		nvrhi::GraphicsState graphicsState;
		nvrhi::DrawArguments drawArgs;

		graphicsState.setFramebuffer(m_framebuffer.handle);
		graphicsState.viewport.addViewportAndScissorRect(
			nvrhi::Viewport(
				0,
				globalData.camera->getWidth(),
				0,
				globalData.camera->getHeight(),
				0,
				1));

		graphicsState.bindings.resize(2);
		graphicsState.bindings[0] = globalData.globalSet;
		graphicsState.bindings[1] = m_instanceBufferSet;

		m_cmd->open();

		nvrhi::utils::ClearDepthStencilAttachment(m_cmd, m_framebuffer.handle, 1.f, 0);
		m_graphicsPipeline->bind(graphicsState, m_framebuffer.handle);

		// Render
		size_t instanceOffset = 0;
		for (auto& [mesh, meshData] : m_renderData) {
			mesh->bindMesh(graphicsState);
			size_t instanceCount = meshData.instanceData.size();
			size_t transMatSize = instanceCount * sizeof(InstanceData);

			memcpy(
				static_cast<uint8_t*>(m_instanceBufferCpuPtr) + instanceOffset * sizeof(InstanceData),
				meshData.instanceData.data(),
				transMatSize);

			uint32_t indicesCount = 0;
			for (auto& subMesh : mesh->getSubMeshes())
				indicesCount += subMesh.indicesCount;
			drawArgs.setVertexCount(indicesCount);
			drawArgs.setStartIndexLocation(0);
			drawArgs.setStartVertexLocation(0);
			drawArgs.setStartInstanceLocation(instanceOffset);
			drawArgs.setInstanceCount(static_cast<uint32_t>(instanceCount));
			m_cmd->setGraphicsState(graphicsState);
			m_cmd->drawIndexed(drawArgs);
			instanceOffset += instanceCount;
		}

		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);
		m_renderData.clear();
	}

	void ForwardPlusDepthRenderer::onViewportResized(uint32_t width, uint32_t height)
	{
		if (m_width == width && m_height == height)
			return;
		if (width == 0 || height == 0)
			return;

		m_width = width;
		m_height = height;
		createFramebuffer();
	}

	void ForwardPlusDepthRenderer::createFramebuffer()
	{
		nvrhi::TextureDesc depthTextureDesc;
		depthTextureDesc
			.setDebugName("PreDepthDepthTexture")
			.setDimension(nvrhi::TextureDimension::Texture2D)
			.setWidth(m_width)
			.setHeight(m_height)
			.setIsRenderTarget(true)
			.setFormat(Application::Get()->getGraphicsContext()->getSupportedDepthFormat());				// TODO fetch from hardware
		m_framebuffer.depthTexture = Application::GetNVRHIDevice()->createTexture(depthTextureDesc);
		nvrhi::FramebufferDesc framebufferDesc;
		framebufferDesc
			.setDepthAttachment(m_framebuffer.depthTexture);
		m_framebuffer.handle = Application::GetNVRHIDevice()->createFramebuffer(framebufferDesc);
	}

}
