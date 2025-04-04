
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

#include "VulkanRenderer.h"

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

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "PaperEngineGame";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "PaperEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = PE_VULKAN_API_VERSION;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		CHECK_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &m_instance));
		PE_CORE_TRACE("Vulkan instance created.");

#ifdef PE_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
		messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
											VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		messengerCreateInfo.pfnUserCallback = &Vulkan_debug_callback;

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = 
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		
		PE_CORE_ASSERT(vkCreateDebugUtilsMessengerEXT, "Failed to get the address of vkCreateDebugUtilsMessengerEXT");

		CHECK_VK_RESULT(vkCreateDebugUtilsMessengerEXT(m_instance, &messengerCreateInfo, NULL, &m_debugMessenger));
		PE_CORE_TRACE("vulkan debug callback created.");
#endif

		// creating the surface
		if (glfwCreateWindowSurface(m_instance, m_windowHandle, nullptr, &m_surface)) {
			PE_CORE_ERROR("[VulkanContext] failed to create window surface");
			exit(-1);
		}

		// selecting device
		m_phys_selector.init(m_instance, m_surface);
		m_queue_family = m_phys_selector.select_device(PE_VULKAN_API_VERSION, VK_QUEUE_GRAPHICS_BIT, true);

		// create vulkan device

		float qPriorities[] = { 1.f };

		VkDeviceQueueCreateInfo qInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = static_cast<uint32_t>(m_queue_family),
			.queueCount = 1,
			.pQueuePriorities = &qPriorities[0]
		};

		std::vector<const char*> devExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
		};

		PE_CORE_ASSERT(m_phys_selector.get_selected().features.tessellationShader == VK_TRUE, "Not support tessellation shader");

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.tessellationShader = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &qInfo,
			.enabledLayerCount = 0,
			.enabledExtensionCount = static_cast<uint32_t>(devExts.size()),
			.ppEnabledExtensionNames = devExts.data(),
			.pEnabledFeatures = &deviceFeatures
		};

		CHECK_VK_RESULT(vkCreateDevice(m_phys_selector.get_selected().physDevice, &deviceCreateInfo, NULL, &m_device));

		PE_CORE_TRACE("vulkan device created.");


		this->create_swapchain();
		this->create_cmd_pool();

		// initialize vma
		VmaAllocatorCreateInfo vmaInfo = {
			.physicalDevice = m_phys_selector.get_selected().physDevice,
			.device = m_device,
			.instance = m_instance,
			.vulkanApiVersion = PE_VULKAN_API_VERSION
		};

		CHECK_VK_RESULT(vmaCreateAllocator(&vmaInfo, &m_allocator));

		// get the present queue
		vkGetDeviceQueue(m_device, m_queue_family, 0, &m_presentQueue);

		// initialize frame rendering barriers
		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		CHECK_VK_RESULT(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence));

		VkSemaphoreCreateInfo semaCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};
		CHECK_VK_RESULT(vkCreateSemaphore(m_device, &semaCreateInfo, nullptr, &m_imageAvailableSem));
		CHECK_VK_RESULT(vkCreateSemaphore(m_device, &semaCreateInfo, nullptr, &m_renderFinishedSem));

		// initialize renderer
		VulkanRenderer::Get().init();
	}

	void VulkanContext::cleanUp()
	{
		if (m_instance == VK_NULL_HANDLE)
			return;

		vkDeviceWaitIdle(m_device);
		vkQueueWaitIdle(m_presentQueue);

		VulkanRenderer::Get().cleanUp();

		// destroy barriers
		vkDestroySemaphore(m_device, m_imageAvailableSem, nullptr);
		m_imageAvailableSem = VK_NULL_HANDLE;
		vkDestroySemaphore(m_device, m_renderFinishedSem, nullptr);
		m_renderFinishedSem = VK_NULL_HANDLE;
		vkDestroyFence(m_device, m_inFlightFence, nullptr);
		m_inFlightFence = VK_NULL_HANDLE;

		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;

		vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
		m_cmdPool = VK_NULL_HANDLE;

		for (const auto& view : m_imageViews) {
			vkDestroyImageView(m_device, view, nullptr);
		}
		m_imageViews.clear();
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
		/// end destruction swapchain


		vkDestroyDevice(m_device, nullptr);

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

#ifdef PE_DEBUG

		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
#endif // PE_DEBUG

		if (m_instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_instance, nullptr);
			m_instance = VK_NULL_HANDLE;
		}
	}

	void VulkanContext::beginFrame()
	{
		CHECK_VK_RESULT(vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX));
		CHECK_VK_RESULT(vkResetFences(m_device, 1, &m_inFlightFence));

		m_current_image_index = UINT32_MAX;
		CHECK_VK_RESULT(vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSem, VK_NULL_HANDLE, &m_current_image_index));

		PE_CORE_ASSERT(m_current_image_index != UINT32_MAX, "wired image index");

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = 0,
			.pInheritanceInfo = nullptr
		};
		CHECK_VK_RESULT(vkResetCommandBuffer(m_cmdBuffers[m_current_image_index], 0));
		CHECK_VK_RESULT(vkBeginCommandBuffer(m_cmdBuffers[m_current_image_index], &beginInfo));

		VulkanRenderer::Get().begin_frame();


		/// temporary clear image
		//VkImageMemoryBarrier toClearBarrier = {
		//	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//	.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		//	.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		//	.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		//	.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//	.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//	.image = m_images[m_current_image_index],
		//	.subresourceRange = {
		//		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		//		.baseMipLevel = 0,
		//		.levelCount = 1,
		//		.baseArrayLayer = 0,
		//		.layerCount = 1
		//	}
		//};
		//vkCmdPipelineBarrier(
		//	m_cmdBuffers[m_current_image_index],
		//	VK_PIPELINE_STAGE_TRANSFER_BIT,
		//	VK_PIPELINE_STAGE_TRANSFER_BIT,
		//	0,
		//	0,
		//	nullptr,
		//	0,
		//	nullptr,
		//	1,
		//	&toClearBarrier);
		//VkClearColorValue clearColor = { 1.f, 0.f, 0.f, 1.f };
		//VkImageSubresourceRange imageRange = {
		//	.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		//	.baseMipLevel = 0,
		//	.levelCount = 1,
		//	.baseArrayLayer = 0,
		//	.layerCount = 1
		//};
		//vkCmdClearColorImage(m_cmdBuffers[m_current_image_index], m_images[m_current_image_index],
		//	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	&clearColor,
		//	1,
		//	&imageRange);
		/// end temp clear image barrier
	}

	void VulkanContext::endFrame()
	{
		VulkanRenderer::Get().end_frame();
		/// temporary translate image
		//VkImageMemoryBarrier toPresentBarrier = {
		//	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//	.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		//	.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		//	.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		//	.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//	.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//	.image = m_images[m_current_image_index],
		//	.subresourceRange = {
		//		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		//		.baseMipLevel = 0,
		//		.levelCount = 1,
		//		.baseArrayLayer = 0,
		//		.layerCount = 1
		//	}
		//};
		//vkCmdPipelineBarrier(
		//	m_cmdBuffers[m_current_image_index],
		//	VK_PIPELINE_STAGE_TRANSFER_BIT,
		//	VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		//	0,
		//	0,
		//	nullptr,
		//	0,
		//	nullptr,
		//	1,
		//	&toPresentBarrier);
		/// end temp translate image barrier


		CHECK_VK_RESULT(vkEndCommandBuffer(m_cmdBuffers[m_current_image_index]));

		VkPipelineStageFlags waitFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_imageAvailableSem,
			.pWaitDstStageMask = &waitFlags,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_cmdBuffers[m_current_image_index],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &m_renderFinishedSem
		};

		CHECK_VK_RESULT(vkQueueSubmit(m_presentQueue, 1, &submitInfo, m_inFlightFence));

		// present
		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = nullptr,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &m_renderFinishedSem,		// wait the render is finish in GPU
			.swapchainCount = 1,
			.pSwapchains = &m_swapchain,
			.pImageIndices = &m_current_image_index
		};

		CHECK_VK_RESULT(vkQueuePresentKHR(m_presentQueue, &presentInfo));
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

	VkCommandBuffer VulkanContext::GetCurrentCmdBuffer()
	{
		PE_CORE_ASSERT(s_instance, "No vulkan instance created.");
		return s_instance->m_cmdBuffers[s_instance->m_current_image_index];
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

	VkPhysicalDevice VulkanContext::GetPhysicalDevice()
	{
		return s_instance->m_phys_selector.get_selected().physDevice;
	}

	uint32_t VulkanContext::GetQueueFamily()
	{
		return s_instance->m_queue_family;
	}

	VkQueue VulkanContext::GetQueue()
	{
		return s_instance->m_presentQueue;
	}

	uint32_t VulkanContext::GetImageCount()
	{
		return static_cast<uint32_t>(s_instance->m_images.size());
	}

	uint32_t VulkanContext::GetCurrentImageIndex()
	{
		return s_instance->m_current_image_index;
	}

	VkSurfaceFormatKHR VulkanContext::GetSwapchainSurfaceFormat()
	{
		return s_instance->m_swapchainFormat;
	}

	std::vector<VkImageView>& VulkanContext::GetSwapchainImageViews()
	{
		return s_instance->m_imageViews;
	}

	VkCommandBuffer VulkanContext::BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = s_instance->m_cmdPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1
		};
		VkCommandBuffer commandBuffer;
		CHECK_VK_RESULT(vkAllocateCommandBuffers(s_instance->m_device, &allocInfo, &commandBuffer));
		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			.pInheritanceInfo = nullptr
		};
		CHECK_VK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
		return commandBuffer;
	}

	void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		CHECK_VK_RESULT(vkEndCommandBuffer(commandBuffer));
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer
		};
		CHECK_VK_RESULT(vkQueueSubmit(s_instance->m_presentQueue, 1, &submitInfo, VK_NULL_HANDLE));
		CHECK_VK_RESULT(vkQueueWaitIdle(s_instance->m_presentQueue));
		vkFreeCommandBuffers(s_instance->m_device, s_instance->m_cmdPool, 1, &commandBuffer);
	}

	void VulkanContext::create_swapchain()
	{
		const auto& surfaceCaps = m_phys_selector.get_selected().surfaceCaps;

		uint32_t num_images = ChooseNumImages(surfaceCaps);
	
		auto surfaceFormat = ChooseSurfaceFormatAndColorSpace(m_phys_selector.get_selected().surfaceFormats);
		// set the swapchain format
		m_swapchainFormat = surfaceFormat;

		auto presentMode = ChoosePresentMode(m_phys_selector.get_selected().presentModes);

		VkSwapchainCreateInfoKHR createInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_surface,
			.minImageCount = num_images,
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = surfaceCaps.currentExtent,
			.imageArrayLayers = 1,
			// used for color attachment, and copy destnation of image
			.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &m_queue_family,
			.preTransform = surfaceCaps.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE
		};

		CHECK_VK_RESULT(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain));

		PE_CORE_TRACE("[VULKAN] Swapchain created.");

		uint32_t num_swapchain = 0;
		CHECK_VK_RESULT(vkGetSwapchainImagesKHR(m_device, m_swapchain, &num_swapchain, NULL));
		PE_CORE_ASSERT(num_swapchain == num_images, "Swapchain image is not equal to image that input.");

		m_images.resize(num_swapchain);
		m_imageViews.resize(num_swapchain);

		// get images in swapchain
		CHECK_VK_RESULT(vkGetSwapchainImagesKHR(m_device, m_swapchain, &num_swapchain, m_images.data()));

		// create image view for swapchain images
		for (uint32_t i = 0; i < num_swapchain; i++) {
			VkImageViewCreateInfo viewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = m_images[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfaceFormat.format,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.levelCount = 1,		// mip map level (No mipmap for presentation)
					.layerCount = 1
				},
			};

			VkImageView imageView;
			CHECK_VK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView));

			m_imageViews[i] = imageView;
		}
	}

	void VulkanContext::create_cmd_pool()
	{
		VkCommandPoolCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,	// the command buffer can be reset
			.queueFamilyIndex = m_queue_family
		};

		CHECK_VK_RESULT(vkCreateCommandPool(m_device, &createInfo, nullptr, &m_cmdPool));

		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_cmdPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(m_images.size())
		};

		m_cmdBuffers.resize(m_images.size());
		CHECK_VK_RESULT(vkAllocateCommandBuffers(m_device, &allocInfo, m_cmdBuffers.data()));

		PE_CORE_TRACE("[VULKAN] primary command buffers created.");
	}
}
