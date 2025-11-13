
#include <vector>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Assert.h>
#include <PaperEngine/core/Logger.h>

#include <PaperEngine/core/GLFWWindow.h>

#include "VulkanGraphicsContext.h"

#ifdef PE_DEBUG
#include <nvrhi/validation.h>
#endif // PE_DEBUG


// 啟用 Vulkan-Hpp 動態載入
#include <vulkan/vulkan.hpp>
namespace vk {
	namespace detail {
		DispatchLoaderDynamic defaultDispatchLoaderDynamic;
	}
}

#pragma region Debug callbacks

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

static VKAPI_ATTR VkBool32 VulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT type,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/
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

#pragma endregion

namespace PaperEngine {
	VulkanGraphicsContext::VulkanGraphicsContext(Window* window) :
		m_window(static_cast<GLFWWindow*>(window))
	{
		PE_CORE_ASSERT(m_window, "VulkanGraphicsContext: Window is null");
		PE_CORE_ASSERT(m_window->getNativeWindow(), "VulkanGraphicsContext: Native window is null");

		m_onBackBufferResizedCallback = []() {
			};
		m_onBackBufferResizingCallback = []() {
			};
	}

	void VulkanGraphicsContext::init()
	{
		//PE_CORE_ASSERT(glfwVulkanSupported(), "GLFW is not support vulkan");

		std::vector<const char*> instanceExtensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (_WIN32)
			"VK_KHR_win32_surface",
#elif defined (__linux__)
			"VK_KHR_xcb_surface",
#endif
#ifdef PE_DEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
		};

#pragma region Vulkan Instance Creation
		{
			vkb::InstanceBuilder builder;

			auto inst_result = builder
				.set_app_name("PaperEngine")
				.set_engine_name("PaperEngine")
				.require_api_version(1, 4, 0)
#ifdef PE_DEBUG
				.request_validation_layers()
				.set_debug_callback(VulkanDebugCallback)
#endif // PE_DEBUG
				.enable_extensions(instanceExtensions)
				.build();

			if (!inst_result) {
				// TODO FATAL
				PE_CORE_ERROR("[Vulkan] Failed to create Vulkan instance: {0}", inst_result.error().message());
				return;
			}
			m_instance.vkbInstance = inst_result.value();
		}
#pragma endregion

#pragma region Surface Creation
		{
			if (!m_window->createSurface(m_instance.vkbInstance.instance, &m_instance.surface))
			{
				// TODO FATAL
				PE_CORE_ERROR("[Vulkan] Failed to create window surface");
				return;
			}
		}
#pragma endregion

		// Features 需要enable
		VkPhysicalDeviceVulkan12Features vulkan12Features{};
		vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		vulkan12Features.timelineSemaphore = VK_TRUE;

		VkPhysicalDeviceVulkan13Features vulkan13Features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.synchronization2 = VK_TRUE, // enable synchronization2
		};

#pragma region GPU Selection
		{

			vkb::PhysicalDeviceSelector selector{ m_instance.vkbInstance };
			auto phys_result = selector
				.set_minimum_version(1, 0)
				.set_surface(m_instance.surface)
				.set_required_features_12(vulkan12Features)
				.set_required_features_13(vulkan13Features)
				.select();
			if (!phys_result) {
				// TODO FATAL
				PE_CORE_ERROR("[Vulkan] Failed to select physical device: {0}", phys_result.error().message());
				return;
			}
			m_instance.physicalDevice = phys_result.value();
		}
#pragma endregion

#pragma region Virtual Device Creation
		{
			vkb::DeviceBuilder deviceBuilder{ m_instance.physicalDevice };
			auto dev_result = deviceBuilder
				.build();
			if (!dev_result) {
				// TODO FATAL
				PE_CORE_ERROR("[Vulkan] Failed to create virtual device: {0}", dev_result.error().message());
				return;
			}
			m_instance.vkbDevice = dev_result.value();
		}
#pragma endregion

#pragma region Get the Queue
		{
			auto graphicsQueueResult = m_instance.vkbDevice.get_queue(vkb::QueueType::graphics);
			if (!graphicsQueueResult) {
				// TODO FATAL
				return;
			}
			m_instance.graphicsQueue = graphicsQueueResult.value();

			m_instance.graphicsQueueIndex = m_instance.vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
			
			auto computeQueueResult = m_instance.vkbDevice.get_queue(vkb::QueueType::compute);
			if (!computeQueueResult) {
				// TODO FATAL
				return;
			}
			m_instance.computeQueue = computeQueueResult.value();
			m_instance.computeQueueIndex = m_instance.vkbDevice.get_queue_index(vkb::QueueType::compute).value();

			auto transferQueueResult = m_instance.vkbDevice.get_queue(vkb::QueueType::transfer);
			if (!transferQueueResult) {
				// TODO FATAL
				return;
			}
			m_instance.transferQueue = transferQueueResult.value();
			m_instance.transferQueueIndex = m_instance.vkbDevice.get_queue_index(vkb::QueueType::transfer).value();

		}
#pragma endregion


#pragma region NVRHI initialization

		vk::detail::defaultDispatchLoaderDynamic.init(
			m_instance.vkbInstance.instance, 
			m_instance.vkbDevice.device);

		nvrhi::vulkan::DeviceDesc deviceDesc{
			.errorCB = &m_NVMsgCallback,
			.physicalDevice = m_instance.physicalDevice.physical_device,
			.device = m_instance.vkbDevice,
			.graphicsQueue = m_instance.graphicsQueue,
			.graphicsQueueIndex = static_cast<int>(m_instance.graphicsQueueIndex),
			.transferQueue = m_instance.transferQueue,
			.transferQueueIndex = static_cast<int>(m_instance.transferQueueIndex),
			.computeQueue = m_instance.computeQueue,
			.computeQueueIndex = static_cast<int>(m_instance.computeQueueIndex),
			.instanceExtensions = instanceExtensions.data(),
			.numInstanceExtensions = instanceExtensions.size(),
		};
		m_instance.device = nvrhi::vulkan::createDevice(deviceDesc);
#ifdef PE_DEBUG
		m_instance.validationDevice = nvrhi::validation::createValidationLayer(m_instance.device);
#endif // PE_DEBUG

#pragma endregion

#pragma region Depth Format Get
		auto getDepthFormat = [this]() {
			std::array<nvrhi::Format, 3> depthFormatCandidates = { 
				nvrhi::Format::D32, 
				nvrhi::Format::D32S8, 
				nvrhi::Format::D24S8};
			for (uint32_t i = 0; i < depthFormatCandidates.size(); i++) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(m_instance.physicalDevice.physical_device, nvrhi::vulkan::convertFormat(depthFormatCandidates[i]), &props);

				if ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
					return depthFormatCandidates[i];
				}
			}
			return nvrhi::Format::UNKNOWN;
			};
		m_instance.depthFormat = getDepthFormat();
		PE_CORE_ASSERT(m_instance.depthFormat != nvrhi::Format::UNKNOWN, "No proper depth format");

#pragma endregion


#pragma region Swapchain creation
		if (!createSwapchain()) {
			// TODO FATAL
			return;
		}
#pragma endregion

#pragma region Initialize Frame in flight synchronization objects
		{
			VkFenceCreateInfo fence_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				//.flags = VK_FENCE_CREATE_SIGNALED_BIT
			};

			m_instance.imageAvailableFences.resize(m_instance.vkbSwapchain.image_count);
			for (uint32_t i = 0; i < m_instance.imageAvailableFences.size(); i++) {

				CHECK_VK_RESULT(vkCreateFence(
					m_instance.vkbDevice.device,
					&fence_info, // fence create info
					nullptr, // allocator
					&m_instance.imageAvailableFences[i]
				));
			}
		}
#pragma endregion

		this->createFramebuffers();

		PE_CORE_TRACE("[Vulkan] Vulkan graphics context initialized successfully!");
	}

	void VulkanGraphicsContext::cleanUp()
	{
		vkDeviceWaitIdle(m_instance.vkbDevice);
		
		m_instance.framebuffers.clear();

		for (const auto& fence : m_instance.imageAvailableFences) {
			vkDestroyFence(m_instance.vkbDevice.device, fence, nullptr);
		}
		m_instance.imageAvailableFences.clear();

		m_instance.swapchainTextures.clear();

		vkb::destroy_swapchain(m_instance.vkbSwapchain);

#ifdef PE_DEBUG
		m_instance.validationDevice = nullptr;
#endif // PE_DEBUG
		m_instance.device = nullptr;

		vkDestroyDevice(m_instance.vkbDevice, nullptr);

		vkDestroySurfaceKHR(m_instance.vkbInstance.instance, m_instance.surface, nullptr);
		m_instance.surface = VK_NULL_HANDLE;
		vkb::destroy_instance(m_instance.vkbInstance);
	}

	bool VulkanGraphicsContext::beginFrame()
	{
		if (m_resizeRequested) {
			if (m_window->getWidth() == 0 || m_window->getHeight() == 0) {
				// 就卡住resizeRequest
				return false;
			}
			this->m_onBackBufferResizingCallback();
			if (!this->createSwapchain()) {
				return false;
			}
			this->createFramebuffers();
			this->m_onBackBufferResizedCallback();
			m_resizeRequested = false;
		}

		VkResult result = VK_SUCCESS;

		result = vkAcquireNextImageKHR(
			m_instance.vkbDevice.device,
			m_instance.vkbSwapchain.swapchain,
			UINT64_MAX, // use the default timeout
			VK_NULL_HANDLE,
			m_instance.imageAvailableFences[m_instance.imageAvailableFenceIndex], // Fence for knowing the image can be write
			&m_instance.swapchainIndex);

		if ((result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)) {
			m_resizeRequested = true;
			return false;
		}

		m_instance.currentImageAvailableFence = m_instance.imageAvailableFences[m_instance.imageAvailableFenceIndex];
		m_instance.imageAvailableFenceIndex = (m_instance.imageAvailableFenceIndex + 1) % m_instance.imageAvailableFences.size();

		if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
			// Successfully acquired the next image
			// 會畫到swapchain image的command需要wait

			return true;
		}

		return false;
	}

	bool VulkanGraphicsContext::present()
	{	
		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 0,
			.pWaitSemaphores = nullptr,
			.swapchainCount = 1,
			.pSwapchains = &m_instance.vkbSwapchain.swapchain,
			.pImageIndices = &m_instance.swapchainIndex,
		};
		
		const VkResult result = vkQueuePresentKHR(
			m_instance.graphicsQueue,
			&presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_resizeRequested = true;
		}

		return result == VK_SUCCESS;
	}

	void VulkanGraphicsContext::waitForSwapchainImageAvailable()
	{
		CHECK_VK_RESULT(vkWaitForFences(
			m_instance.vkbDevice.device,
			1, // one fence
			&m_instance.currentImageAvailableFence,
			VK_TRUE, // wait for the fence to be signaled
			UINT64_MAX // no timeout
		));
		CHECK_VK_RESULT(vkResetFences(
			m_instance.vkbDevice.device,
			1, // one fence
			&m_instance.currentImageAvailableFence
		));
	}

	uint32_t VulkanGraphicsContext::getSwapchainCount()
	{
		return m_instance.vkbSwapchain.image_count;
	}

	uint32_t VulkanGraphicsContext::getSwapchainIndex() const
	{
		return m_instance.swapchainIndex;
	}

	nvrhi::TextureHandle VulkanGraphicsContext::getCurrentSwapchainTexture()
	{
		return m_instance.swapchainTextures[m_instance.swapchainIndex];
	}

	void VulkanGraphicsContext::setOnBackBufferResizingCallback(const std::function<void()>& callback)
	{
		m_onBackBufferResizingCallback = callback;
	}

	void VulkanGraphicsContext::setOnBackBufferResizedCallback(const std::function<void()>& callback)
	{
		m_onBackBufferResizedCallback = callback;
	}

	nvrhi::IDevice* VulkanGraphicsContext::getNVRhiDevice() const
	{
#ifdef PE_DEBUG
		if (m_instance.validationDevice)
			return m_instance.validationDevice;
#endif // PE_DEBUG

		return m_instance.device;
	}

	nvrhi::FramebufferHandle VulkanGraphicsContext::getCurrentFramebuffer()
	{
		return m_instance.framebuffers[m_instance.swapchainIndex].framebuffer;
	}

	nvrhi::Format VulkanGraphicsContext::getSupportedDepthFormat()
	{
		return m_instance.depthFormat;
	}

	bool VulkanGraphicsContext::createSwapchain()
	{
		vkDeviceWaitIdle(m_instance.vkbDevice.device);

		if (!m_instance.imageAvailableFences.empty()) {
			vkResetFences(m_instance.vkbDevice, static_cast<uint32_t>(m_instance.imageAvailableFences.size()), m_instance.imageAvailableFences.data());
		}

		m_instance.swapchainTextures.clear();
		VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};
		auto swapResult = vkb::SwapchainBuilder(m_instance.vkbDevice)
			.set_desired_format(surfaceFormat)
			.set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.set_old_swapchain(m_instance.vkbSwapchain)
			.build();
		if (!swapResult) {
			PE_CORE_ERROR("[Vulkan] Failed to create swapchain: {0}", swapResult.error().message());
			return false;
		}
		vkb::destroy_swapchain(m_instance.vkbSwapchain);
		m_instance.vkbSwapchain = swapResult.value();

		auto swapchainImages = m_instance.vkbSwapchain.get_images().value();

		{
			auto textureDesc = nvrhi::TextureDesc()
				.setDebugName("Swap chain image")
				.setInitialState(nvrhi::ResourceStates::Present)
				.setKeepInitialState(true)
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::Format::RGBA8_UNORM)			// TODO 改一下configuable
				.setWidth(m_window->getWidth())
				.setHeight(m_window->getHeight())
				.setIsRenderTarget(true);
			for (const auto& image : swapchainImages) {
				auto texture = m_instance.device->createHandleForNativeTexture(nvrhi::ObjectTypes::VK_Image, image, textureDesc);
				m_instance.swapchainTextures.push_back(texture);
			}
		}

		PE_CORE_TRACE("[VulkanGraphicsContext] Swapchain recreated with size {}x{}", m_instance.vkbSwapchain.extent.width, m_instance.vkbSwapchain.extent.height);

		return true;
	}

	void VulkanGraphicsContext::createFramebuffers()
	{
		m_instance.framebuffers.clear();
		for (auto& texture : m_instance.swapchainTextures) {
			auto depthTextureDesc = nvrhi::TextureDesc()
				.setWidth(m_instance.vkbSwapchain.extent.width)
				.setHeight(m_instance.vkbSwapchain.extent.height)
				.setFormat(m_instance.depthFormat)
				.setIsRenderTarget(true);
			auto depthTexture = m_instance.device->createTexture(depthTextureDesc);
			auto framebufferDesc = nvrhi::FramebufferDesc()
				.addColorAttachment(texture)
				.setDepthAttachment(depthTexture);
			m_instance.framebuffers.push_back({
				depthTexture,
				m_instance.device->createFramebuffer(framebufferDesc)
				});
		}
	}

}
