
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

#include <vulkan/vulkan.hpp>

#include "VulkanSceneRenderer.h"
#include "VulkanMeshRenderer.h"

namespace PaperEngine {

	VulkanContext* VulkanContext::s_instance{ nullptr };

	VulkanContext::VulkanContext(GLFWwindow* handle)
		: m_windowHandle(handle)
	{
		PE_CORE_ASSERT(!s_instance, "Only one context can be created.");
		s_instance = this;
	}

	VulkanContext::~VulkanContext()
	{
		this->cleanUp();
		s_instance = nullptr;
	}

	static const char* Vulkan_get_debug_type(VkDebugUtilsMessageTypeFlagsEXT type) {
		switch (type)
		{
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
			return "General";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
			return "Validation";
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
			return "Performance";
		default:
			break;
		}
		return "Error type";
	}

	static VKAPI_ATTR VkBool32 Vulkan_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		switch (severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			PE_CORE_TRACE("[VULKAN] {}: {}", Vulkan_get_debug_type(type), pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			PE_CORE_INFO("[VULKAN] {}: {}", Vulkan_get_debug_type(type), pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			PE_CORE_WARN("[VULKAN] {}: {}", Vulkan_get_debug_type(type), pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			PE_CORE_ERROR("[VULKAN] {}: {}", Vulkan_get_debug_type(type), pCallbackData->pMessage);
			break;
		default:
			break;
		}

		PE_CORE_TRACE("Object addresses:");
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
			PE_CORE_TRACE("{:4}: 0x{:016x} ", i, pCallbackData->pObjects[i].objectHandle);
		}

		return VK_FALSE;
	}

	void VulkanContext::init()
	{
		PE_CORE_ASSERT(glfwVulkanSupported(), "GLFW is not support vulkan");

		std::vector<const char*> layers = {
#ifdef PE_DEBUG
			"VK_LAYER_KHRONOS_validation"
#endif
		};

		std::vector<const char*> extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
			"VK_KHR_win32_surface",
#elif defined (__linux__)
			"VK_KHR_xcb_surface",
#endif
#ifdef PE_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
		};

		// instance creation
		{
			vkb::InstanceBuilder builder;
			auto inst_ret = builder
				.set_engine_name("PaperEngine")
				.set_app_name("PaperEngine App")
				.require_api_version(1, 3)
#ifdef PE_DEBUG
				.request_validation_layers()
				.set_debug_callback(Vulkan_debug_callback)
#endif // PE_DEBUG
				.build();
			if (!inst_ret) {
				PE_CORE_ERROR("[VulkanContext] Failed to create instance.");
			}
			m_instance = inst_ret.value();
		}

		// surface creation
		{
			// creating the surface
			if (glfwCreateWindowSurface(m_instance, m_windowHandle, nullptr, &m_surface)) {
				PE_CORE_ERROR("[VulkanContext] failed to create window surface");
				exit(-1);
			}
		}

		// physical device selector
		std::vector<const char*> device_extensions = {
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME
		};

		VkPhysicalDeviceVulkan12Features vulkan12_features{};
		vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12_features.timelineSemaphore = VK_TRUE;
		
		VkPhysicalDeviceVulkan13Features vulkan13_features{};
		vulkan13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		vulkan13_features.synchronization2 = VK_TRUE;

		{
			vkb::PhysicalDeviceSelector selector{ m_instance };
			auto phys_ret = selector
				.set_surface(m_surface)
				.require_dedicated_transfer_queue()
				.add_required_extensions(device_extensions)
				.add_required_extension_features(vulkan12_features)
				.add_required_extension_features(vulkan13_features)
				.select();
			if (!phys_ret) {
				PE_CORE_ERROR("[VulkanContext] Failed to select gpu.");
				exit(-1);
			}
			m_phys_device = phys_ret.value();
		}

#pragma region Depth Buffer Format Selection
		auto getDepthFormat = [this]() {
			std::array<VkFormat, 3> depthFormatCandidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
			for (uint32_t i = 0; i < depthFormatCandidates.size(); i++) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(m_phys_device.physical_device, depthFormatCandidates[i], &props);

				if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					return depthFormatCandidates[i];
				}
			}
			return VK_FORMAT_UNDEFINED;
			};
		m_depthFormat = getDepthFormat();
		PE_CORE_ASSERT(m_depthFormat != VK_FORMAT_UNDEFINED, "No proper depth format");
#pragma endregion


		{
			vkb::DeviceBuilder builder{ m_phys_device };
			auto dev_ret = builder.build();
			if (!dev_ret) {
				PE_CORE_ERROR("[VulkanContext] Failed to create vulkan device.");
				exit(-1);
			}
			m_device = dev_ret.value();
			PE_CORE_TRACE("vulkan device created.");
		}
		{
			m_graphics_queue = m_device.get_queue(vkb::QueueType::graphics).value();
			m_graphics_queue_index = m_device.get_queue_index(vkb::QueueType::graphics).value();
			m_present_queue = m_device.get_queue(vkb::QueueType::present).value();
		}

		// find depth format

		this->create_swapchain();

		// initialize frame rendering barriers
		// frame synchronization
		{
			VkSemaphoreCreateInfo create_info = {
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};
			VkFenceCreateInfo fence_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT
			};
			m_image_available_sems.resize((size_t)m_swapchain.image_count + 1);
			m_frames_in_flight_fences.resize((size_t)m_swapchain.image_count + 1);
			m_render_finish_sems.resize((size_t)m_swapchain.image_count + 1);
			for (uint32_t i = 0; i < m_image_available_sems.size(); i++) {
				CHECK_VK_RESULT(vkCreateSemaphore(m_device, &create_info, nullptr, &m_image_available_sems[i]));
				CHECK_VK_RESULT(vkCreateSemaphore(m_device, &create_info, nullptr, &m_render_finish_sems[i]));
				CHECK_VK_RESULT(vkCreateFence(m_device, &fence_info, nullptr, &m_frames_in_flight_fences[i]));
			}

		}

#pragma region Initialize VMA
		// initialize vma
		VmaAllocatorCreateInfo vmaInfo = {
			.physicalDevice = m_phys_device,
			.device = m_device,
			.instance = m_instance,
			.vulkanApiVersion = PE_VULKAN_API_VERSION
		};

		CHECK_VK_RESULT(vmaCreateAllocator(&vmaInfo, &m_allocator));
#pragma endregion

#pragma region Create command pool
		VkCommandPoolCreateInfo pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_graphics_queue_index,
		};
		CHECK_VK_RESULT(vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool));
#pragma endregion

		m_setManager = CreateRef<VulkanDescriptorSetManager>();

		m_cmdManager = CreateRef<VulkanCommandBufferManager>();

		// initialize scene renderer static
		VulkanSceneRenderer::Init();
		VulkanMeshRenderer::Init();
	}

	void VulkanContext::cleanUp()
	{
		if (m_instance == VK_NULL_HANDLE)
			return;

		vkDeviceWaitIdle(m_device);

		VulkanMeshRenderer::CleanUp();
		VulkanSceneRenderer::CleanUp();

		m_cmdManager.reset();
		m_setManager.reset();

		vkDestroyCommandPool(m_device, m_command_pool, nullptr);
		m_command_pool = VK_NULL_HANDLE;

		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;


		for (uint32_t i = 0; i < m_image_available_sems.size(); i++) {
			vkDestroySemaphore(m_device, m_image_available_sems[i], nullptr);
			vkDestroySemaphore(m_device, m_render_finish_sems[i], nullptr);
			vkDestroyFence(m_device, m_frames_in_flight_fences[i], nullptr);
		}
		m_image_available_sems.clear();
		m_frames_in_flight_fences.clear();

		//m_swapchain_textures.clear();
		m_swapchain_textures.clear();
		vkb::destroy_swapchain(m_swapchain);
		m_swapchain = {};

		vkb::destroy_device(m_device);
		m_device = {};
		vkb::destroy_surface(m_instance, m_surface);
		m_surface = VK_NULL_HANDLE;
		vkb::destroy_instance(m_instance);
		m_instance = {};

	}

	bool VulkanContext::beginFrame()
	{
		if (m_resizeRequested) {
			PE_CORE_INFO("Swapchain resize:");
			if (!this->create_swapchain()) {
				PE_CORE_ASSERT(false, "Failed to recreate swapchain.");
				return false;
			}
			m_resizeRequested = false;
		}
		// TODO: reset this frame command pool
		CHECK_VK_RESULT(vkResetFences(m_device, 1, &m_frames_in_flight_fences[m_frames_in_flight_index]));

		VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_image_available_sems[m_image_ava_sem_index], VK_NULL_HANDLE, &m_current_image_index);

		switch (result)
		{
		case VK_SUCCESS:
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			m_resizeRequested = true;
			return false;
		default:
			PE_CORE_ERROR("Unsupport error during acquiring next image.");
			return false;
		}
		return true;
	}

	void VulkanContext::endFrame()
	{

		VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit_info = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_image_available_sems[m_image_ava_sem_index],
			.pWaitDstStageMask = &wait_stages,
			.commandBufferCount = static_cast<uint32_t>(m_submit_buffers.size()),
			.pCommandBuffers = m_submit_buffers.data(),
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_render_finish_sems[m_render_finish_sem_index]
		};
		CHECK_VK_RESULT(vkQueueSubmit(m_graphics_queue, 1, &submit_info, m_frames_in_flight_fences[m_frames_in_flight_index]));
		// clear the submit buffers
		m_submit_buffers.clear();


		m_image_ava_sem_index = (m_image_ava_sem_index + 1) % (uint32_t)m_image_available_sems.size();
		m_frames_in_flight_index = (m_frames_in_flight_index + 1) % (uint32_t)m_frames_in_flight_fences.size();

		VkPresentInfoKHR present_info = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_render_finish_sems[m_render_finish_sem_index],		// wait the render is finish in GPU
			.swapchainCount = 1,
			.pSwapchains = &m_swapchain.swapchain,
			.pImageIndices = &m_current_image_index
		};

		VkResult result = vkQueuePresentKHR(m_present_queue, &present_info);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_resizeRequested = true;
		}
		m_render_finish_sem_index = (m_render_finish_sem_index + 1) % (uint32_t)m_render_finish_sems.size();

		CHECK_VK_RESULT(vkWaitForFences(m_device, 1, &m_frames_in_flight_fences[m_frames_in_flight_index], VK_TRUE, UINT64_MAX));
	}

	void VulkanContext::executeCommandBuffer(CommandBufferHandle cmd)
	{
		this->m_cmdManager->executeCommandBuffer(cmd);
	}

	void VulkanContext::executeCommandBuffers(uint32_t count, CommandBufferHandle* cmds)
	{
		this->m_cmdManager->executeCommandBuffers(count, cmds);
	}

	uint32_t VulkanContext::get_swapchain_image_count()
	{
		PE_CORE_ASSERT(m_swapchain, "Swapchain was not created.");
		return m_swapchain.image_count;
	}

	uint32_t VulkanContext::get_current_swapchain_index()
	{
		return m_current_image_index;
	}

	TextureHandle VulkanContext::get_swapchain_texture(uint32_t swapchainIndex)
	{
		PE_CORE_ASSERT(swapchainIndex < m_swapchain_textures.size(), "Invalid swapchain index");
		return m_swapchain_textures[swapchainIndex];
	}

	static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& surfaceFormats) {
		for (const auto& format : surfaceFormats) {
			if ((format.format == VK_FORMAT_B8G8R8A8_UNORM) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
				return format;
			}
		}
		return surfaceFormats[0];
	}

	static uint32_t ChooseNumImages(const VkSurfaceCapabilitiesKHR& capabilities) {
		uint32_t requestedNumImages = capabilities.minImageCount + 1;

		int finalNumImages = 0;

		if ((capabilities.maxImageCount > 0) && (requestedNumImages > capabilities.maxImageCount)) {
			finalNumImages = capabilities.maxImageCount;
		}
		else {
			finalNumImages = requestedNumImages;
		}

		return finalNumImages;
	}

	static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
		for (const auto& mode : presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkDevice VulkanContext::GetDevice()
	{
		PE_CORE_ASSERT(s_instance, "No vulkan instance created.");
		return s_instance->m_device;
	}

	VkInstance VulkanContext::GetInstance()
	{
		return s_instance->m_instance;
	}

	VmaAllocator VulkanContext::GetAllocator()
	{
		return s_instance->m_allocator;
	}

	uint32_t VulkanContext::GetImageCount()
	{
		return s_instance->m_swapchain.image_count;
	}

	uint32_t VulkanContext::GetCurrentImageIndex()
	{
		return s_instance->m_current_image_index;
	}

	bool VulkanContext::create_swapchain()
	{
		m_swapchain_textures.clear();
		vkb::SwapchainBuilder builder(m_device);

		VkSurfaceFormatKHR swapchain_format = {
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};

		auto swap_ret = builder
			.set_desired_format(swapchain_format)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.set_old_swapchain(m_swapchain)
			.build();
		if (!swap_ret) {
			PE_CORE_ERROR("[VulkanContext] Failed to create swapchain: {}: {}", swap_ret.vk_result(), swap_ret.error().message());
			return false;
		}
		vkb::destroy_swapchain(m_swapchain);
		m_swapchain = swap_ret.value();

		auto images = m_swapchain.get_images().value();

		{
			// TODO: create my own swapchain textures
			m_swapchain_textures.resize(images.size());
			TextureSpecification swapchainTextureSpec;
			swapchainTextureSpec.width = m_swapchain.extent.width;
			swapchainTextureSpec.height = m_swapchain.extent.height;
			swapchainTextureSpec.format = TextureFormat::Present;
			swapchainTextureSpec.isRenderTarget = true;
			swapchainTextureSpec.type = Texture2D;
			for (uint32_t i = 0; i < images.size(); i++) {
				m_swapchain_textures[i] = CreateRef<VulkanTexture>(images[i], swapchainTextureSpec);
			}
		}

		return true;
	}

	void VulkanContext::present()
	{
	}
}
