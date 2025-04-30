#include "VulkanImGuiLayer.h"

#include <Platform/Vulkan/VulkanUtils.h>
#include <Platform/Vulkan/VulkanSceneRenderer.h>

#include <PaperEngine/events/KeyEvent.h>
#include <PaperEngine/events/MouseEvent.h>

#include <backends/imgui_impl_glfw.h>
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int keycode, int scancode);

namespace PaperEngine {
	
	bool VulkanImGuiLayer::begin_frame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();

		return true;
	}
	
	void VulkanImGuiLayer::end_frame()
	{
		ImGui::Render();
		ImDrawData* draw_data = ImGui::GetDrawData();

		if (m_width != VulkanContext::GetSwapchain().extent.width || m_height != VulkanContext::GetSwapchain().extent.height) {
			create_framebuffers();
		}

		VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();
		cmd->open();
		cmd->setTextureState(VulkanContext::GetSwapchainTexture(VulkanContext::GetCurrentImageIndex()), TextureState::ColorAttachment);
		VkRenderPassBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = m_renderPass,
			.framebuffer = m_framebuffers[VulkanContext::GetCurrentImageIndex()],
			.renderArea = {
				.extent = VulkanContext::GetSwapchain().extent,
			},
			.clearValueCount = 0
		};
		vkCmdBeginRenderPass(cmd->get_handle(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(draw_data, cmd->get_handle());
		vkCmdEndRenderPass(cmd->get_handle());
		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);

	}

	void VulkanImGuiLayer::on_attach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;


#pragma region Vulkan IMGUI Initialization

		VkResult err = VK_SUCCESS;
		// Create Descriptor Pool
		// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
		{
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 0;
			for (VkDescriptorPoolSize& pool_size : pool_sizes)
				pool_info.maxSets += pool_size.descriptorCount;
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			CHECK_VK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice(), &pool_info, nullptr, &m_descPool));
		}

		// create render pass
		{
			VkAttachmentDescription attachment = {
				.format = VulkanContext::GetSwapchain().image_format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,			// store the data
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
			VkAttachmentReference colorAttRef = {
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
			VkSubpassDescription subpass = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttRef,
			};
			VkRenderPassCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.attachmentCount = 1,
				.pAttachments = &attachment,
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 0,
				.pDependencies = nullptr
			};
			CHECK_VK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice(), &createInfo, nullptr, &m_renderPass));
		}

		create_framebuffers();

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = VulkanContext::GetInstance();
		initInfo.PhysicalDevice = VulkanContext::GetPhysicalDevice().physical_device;
		initInfo.Device = VulkanContext::GetDevice();
		initInfo.QueueFamily = VulkanContext::GetGraphicsQueueIndex();
		initInfo.Queue = VulkanContext::GetGraphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = m_descPool;
		initInfo.RenderPass = m_renderPass;
		initInfo.Subpass = 0;
		initInfo.MinImageCount = VulkanContext::GetImageCount();
		initInfo.ImageCount = VulkanContext::GetImageCount();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		initInfo.CheckVkResultFn = nullptr; // TODO
		ImGui_ImplVulkan_Init(&initInfo);
#pragma endregion


	}

	void VulkanImGuiLayer::on_detach()
	{
		vkDeviceWaitIdle(VulkanContext::GetDevice());

		ImGui_ImplVulkan_Shutdown();

		for (auto framebuffer : m_framebuffers) {
			vkDestroyFramebuffer(VulkanContext::GetDevice(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(VulkanContext::GetDevice(), m_renderPass, nullptr);
		m_renderPass = VK_NULL_HANDLE;

		vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_descPool, nullptr);
		m_descPool = VK_NULL_HANDLE;
	}

	void VulkanImGuiLayer::on_update(Timestep delta_time)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = delta_time.to_seconds();
	}

	void VulkanImGuiLayer::on_event(Event& e)
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
			return false;
			});
		dispatcher.dispatch<KeyReleasedEvent>([](KeyReleasedEvent& e) {

			ImGuiIO& io = ImGui::GetIO();
			ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(e.get_key_code(), e.get_scancode());
			io.AddKeyEvent(imgui_key, false);
			io.SetKeyEventNativeData(imgui_key, e.get_key_code(), e.get_scancode()); // To support legacy indexing (<1.87 user code)

			return false;
			});
		dispatcher.dispatch<MouseButtonPressedEvent>([](MouseButtonPressedEvent& e) {

			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(e.GetMouseButton(), true);
			if (e.GetMouseButton() == Mouse::ButtonLeft && 
				!ImGui::IsAnyItemHovered() && 
				!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			{
				// Left mouse click happened on a blank area (not on any ImGui window or item)
				return false;
			}
			return true;
			});
		dispatcher.dispatch<KeyTypedEvent>([](KeyTypedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter(e.get_key_code());
			return false;
			});
		dispatcher.dispatch<MouseButtonReleasedEvent>([](MouseButtonReleasedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(e.GetMouseButton(), false);
			return false;
			});
		dispatcher.dispatch<MouseMovedEvent>([](MouseMovedEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMousePosEvent(e.GetX(), e.GetY());
			return false;
			});
		dispatcher.dispatch<MouseScrolledEvent>([](MouseScrolledEvent& e) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseWheelEvent(e.GetXOffset(), e.GetYOffset());
			return false;
			});
	}

	void VulkanImGuiLayer::create_framebuffers()
	{
		for (auto framebuffer : m_framebuffers) {
			vkDestroyFramebuffer(VulkanContext::GetDevice(), framebuffer, nullptr);
		}

		m_framebuffers.resize(VulkanContext::GetImageCount());
		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_renderPass,
			.attachmentCount = 1,
			.width = VulkanContext::GetSwapchain().extent.width,
			.height = VulkanContext::GetSwapchain().extent.height,
			.layers = 1
		};
		m_width = VulkanContext::GetSwapchain().extent.width;
		m_height = VulkanContext::GetSwapchain().extent.height;
		for (uint32_t i = 0; i < VulkanContext::GetImageCount(); i++) {
			VkImageView view = VulkanContext::GetSwapchainTexture(i)->get_image_view();
			createInfo.pAttachments = &view;
			CHECK_VK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice(), &createInfo, nullptr, &m_framebuffers[i]));
		}
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = static_cast<float>(m_width);
		io.DisplaySize.y = static_cast<float>(m_height);
	}

}
