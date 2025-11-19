
#include <vector>
#include <iostream>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Assert.h>
#include <PaperEngine/core/Logger.h>
#include <PaperEngine/debug/Instrumentor.h>

#include "VulkanGraphicsContext.h"

#ifdef PE_DEBUG
#include <nvrhi/validation.h>
#endif // PE_DEBUG
#include <nvrhi/vulkan.h>

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
		//PE_DEBUGBREAK();
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
			.dynamicRendering = VK_TRUE, // enable dynamic rendering
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
				// use graphics queue as compute queue
					
				m_instance.computeQueue = m_instance.graphicsQueue;
				m_instance.computeQueueIndex = m_instance.graphicsQueueIndex;
				PE_CORE_WARN("faild to get independent compute queue, use graphics queue instead.");
			} else {
				m_instance.computeQueue = computeQueueResult.value();
				m_instance.computeQueueIndex = m_instance.vkbDevice.get_queue_index(vkb::QueueType::compute).value();
			}

			auto transferQueueResult = m_instance.vkbDevice.get_queue(vkb::QueueType::transfer);
			if (!transferQueueResult) {
				m_instance.transferQueue = m_instance.graphicsQueue;
				m_instance.transferQueueIndex = m_instance.graphicsQueueIndex;
				PE_CORE_WARN("faild to get independent transfer queue, use graphics queue instead.");
			} else {
				m_instance.transferQueue = transferQueueResult.value();
				m_instance.transferQueueIndex = m_instance.vkbDevice.get_queue_index(vkb::QueueType::transfer).value();
			}
		}
#pragma endregion


#pragma region NVRHI initialization

		vk::detail::defaultDispatchLoaderDynamic.init(
			m_instance.vkbInstance.instance, 
			m_instance.vkbDevice.device);

		nvrhi::vulkan::DeviceDesc deviceDesc{
			.errorCB = &m_NVMsgCallback,
			.instance = m_instance.vkbInstance.instance,
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
			.vulkanLibraryName = ""
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
			throw std::runtime_error("failed to create swapchain.");
		}
#pragma endregion

		this->createFramebuffers();

		this->createFrameContentObjects();

		PE_CORE_TRACE("[Vulkan] Vulkan graphics context initialized successfully!");
	}

	void VulkanGraphicsContext::cleanUp()
	{
		vkDeviceWaitIdle(m_instance.vkbDevice);
		
		this->destroyFrameContentObjects();

		m_instance.framebuffers.clear();

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

		const auto& current_frame_content = m_frame_contents[m_current_frame_index];

		m_instance.device->waitEventQuery(current_frame_content.fence);
		m_instance.device->resetEventQuery(current_frame_content.fence);

		result = vkAcquireNextImageKHR(
			m_instance.vkbDevice.device,
			m_instance.vkbSwapchain.swapchain,
			UINT64_MAX, // use the default timeout
			current_frame_content.image_available_semaphore,
			VK_NULL_HANDLE, // Fence for knowing the image can be write
			&m_instance.swapchainIndex);

		if ((result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)) {
			m_resizeRequested = true;
			return false;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			return false;
		}

		// Successfully acquired the next image
		// 會畫到swapchain image的command需要wait
		return true;
	}

	bool VulkanGraphicsContext::present()
	{
		auto vk_device = static_cast<nvrhi::vulkan::IDevice*>(m_instance.device.Get());

		// signal render finished semaphore
		//vk_device->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, [insert render finished semaphore], 0);

		const auto& current_frame_content = m_frame_contents[m_current_frame_index];

		PE_PROFILE_FUNCTION();
		{
			// 需要確定device沒有
			PE_PROFILE_SCOPE("Commit and wait main command buffer");
			// don't care the value because it is binary semaphore
			vk_device->queueWaitForSemaphore(nvrhi::CommandQueue::Graphics, current_frame_content.image_available_semaphore, 0);
			vk_device->queueSignalSemaphore(nvrhi::CommandQueue::Graphics, current_frame_content.render_finished_semaphore, 0);
			m_instance.device->executeCommandList(current_frame_content.cmd);
			m_instance.device->setEventQuery(current_frame_content.fence, nvrhi::CommandQueue::Graphics);
		}

		VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &current_frame_content.render_finished_semaphore,
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

		m_current_frame_index = (m_current_frame_index + 1) % static_cast<uint32_t>(m_frame_contents.size());

		return result == VK_SUCCESS;
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

	nvrhi::CommandListHandle VulkanGraphicsContext::getMainCommandList()
	{
		return m_frame_contents[m_current_frame_index].cmd;
	}

	bool VulkanGraphicsContext::createSwapchain()
	{
		vkDeviceWaitIdle(m_instance.vkbDevice.device);

		m_instance.swapchainTextures.clear();
		VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};
		auto swapResult = vkb::SwapchainBuilder(m_instance.vkbDevice)
			.set_desired_format(surfaceFormat)
			.set_desired_extent(m_window->getWidth(), m_window->getHeight())
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
				.setClearValue(nvrhi::Color(0.0f, 0.0f, 0.0f, 0.0f))
				.setUseClearValue(true)
				.setDimension(nvrhi::TextureDimension::Texture2D)
				.setFormat(nvrhi::VulkanFormatFromVkFormat(m_instance.vkbSwapchain.image_format))
				.setWidth(m_instance.vkbSwapchain.extent.width)
				.setHeight(m_instance.vkbSwapchain.extent.height)
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
				.setDebugName("Main_framebuffer_Depth_buffer")
				.setIsRenderTarget(true)
				.setInitialState(nvrhi::ResourceStates::DepthWrite)
				.setKeepInitialState(true)
				.setWidth(m_instance.vkbSwapchain.extent.width)
				.setHeight(m_instance.vkbSwapchain.extent.height)
				.setFormat(m_instance.depthFormat);
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

	void VulkanGraphicsContext::createFrameContentObjects()
	{

		m_frame_contents.resize(m_instance.vkbSwapchain.image_count);
		for (auto& frame_content : m_frame_contents)
		{
			VkFenceCreateInfo fence_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			frame_content.fence = m_instance.device->createEventQuery();
			VkSemaphoreCreateInfo sema_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
			vkCreateSemaphore(m_instance.vkbDevice.device, &sema_info, nullptr, &frame_content.image_available_semaphore);
			vkCreateSemaphore(m_instance.vkbDevice.device, &sema_info, nullptr, &frame_content.render_finished_semaphore);
			frame_content.cmd = m_instance.device->createCommandList();
		}
	}
	
	void VulkanGraphicsContext::destroyFrameContentObjects()
	{
		for (auto& frame_content : m_frame_contents)
		{
			vkDestroySemaphore(m_instance.vkbDevice.device, frame_content.image_available_semaphore, nullptr);
			vkDestroySemaphore(m_instance.vkbDevice.device, frame_content.render_finished_semaphore, nullptr);
		}
		m_frame_contents.clear();
	}
}

namespace nvrhi
{

	struct FormatMapping
	{
		nvrhi::Format rhiFormat;
		VkFormat vkFormat;
	};

	static const std::array<FormatMapping, size_t(nvrhi::Format::COUNT)> c_FormatMap = { {
		{ Format::UNKNOWN,           VK_FORMAT_UNDEFINED                },
		{ Format::R8_UINT,           VK_FORMAT_R8_UINT                  },
		{ Format::R8_SINT,           VK_FORMAT_R8_SINT                  },
		{ Format::R8_UNORM,          VK_FORMAT_R8_UNORM                 },
		{ Format::R8_SNORM,          VK_FORMAT_R8_SNORM                 },
		{ Format::RG8_UINT,          VK_FORMAT_R8G8_UINT                },
		{ Format::RG8_SINT,          VK_FORMAT_R8G8_SINT                },
		{ Format::RG8_UNORM,         VK_FORMAT_R8G8_UNORM               },
		{ Format::RG8_SNORM,         VK_FORMAT_R8G8_SNORM               },
		{ Format::R16_UINT,          VK_FORMAT_R16_UINT                 },
		{ Format::R16_SINT,          VK_FORMAT_R16_SINT                 },
		{ Format::R16_UNORM,         VK_FORMAT_R16_UNORM                },
		{ Format::R16_SNORM,         VK_FORMAT_R16_SNORM                },
		{ Format::R16_FLOAT,         VK_FORMAT_R16_SFLOAT               },
		{ Format::BGRA4_UNORM,       VK_FORMAT_A4R4G4B4_UNORM_PACK16    }, // this format matches the bit layout of DXGI_FORMAT_B4G4R4A4_UNORM
		{ Format::B5G6R5_UNORM,      VK_FORMAT_B5G6R5_UNORM_PACK16      },
		{ Format::B5G5R5A1_UNORM,    VK_FORMAT_B5G5R5A1_UNORM_PACK16    },
		{ Format::RGBA8_UINT,        VK_FORMAT_R8G8B8A8_UINT            },
		{ Format::RGBA8_SINT,        VK_FORMAT_R8G8B8A8_SINT            },
		{ Format::RGBA8_UNORM,       VK_FORMAT_R8G8B8A8_UNORM           },
		{ Format::RGBA8_SNORM,       VK_FORMAT_R8G8B8A8_SNORM           },
		{ Format::BGRA8_UNORM,       VK_FORMAT_B8G8R8A8_UNORM           },
		{ Format::BGRX8_UNORM,       VK_FORMAT_UNDEFINED                }, // Not supported on Vulkan
		{ Format::SRGBA8_UNORM,      VK_FORMAT_R8G8B8A8_SRGB            },
		{ Format::SBGRA8_UNORM,      VK_FORMAT_B8G8R8A8_SRGB            },
		{ Format::SBGRX8_UNORM,      VK_FORMAT_UNDEFINED                }, // Not supported on Vulkan
		{ Format::R10G10B10A2_UNORM, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
		{ Format::R11G11B10_FLOAT,   VK_FORMAT_B10G11R11_UFLOAT_PACK32  },
		{ Format::RG16_UINT,         VK_FORMAT_R16G16_UINT              },
		{ Format::RG16_SINT,         VK_FORMAT_R16G16_SINT              },
		{ Format::RG16_UNORM,        VK_FORMAT_R16G16_UNORM             },
		{ Format::RG16_SNORM,        VK_FORMAT_R16G16_SNORM             },
		{ Format::RG16_FLOAT,        VK_FORMAT_R16G16_SFLOAT            },
		{ Format::R32_UINT,          VK_FORMAT_R32_UINT                 },
		{ Format::R32_SINT,          VK_FORMAT_R32_SINT                 },
		{ Format::R32_FLOAT,         VK_FORMAT_R32_SFLOAT               },
		{ Format::RGBA16_UINT,       VK_FORMAT_R16G16B16A16_UINT        },
		{ Format::RGBA16_SINT,       VK_FORMAT_R16G16B16A16_SINT        },
		{ Format::RGBA16_FLOAT,      VK_FORMAT_R16G16B16A16_SFLOAT      },
		{ Format::RGBA16_UNORM,      VK_FORMAT_R16G16B16A16_UNORM       },
		{ Format::RGBA16_SNORM,      VK_FORMAT_R16G16B16A16_SNORM       },
		{ Format::RG32_UINT,         VK_FORMAT_R32G32_UINT              },
		{ Format::RG32_SINT,         VK_FORMAT_R32G32_SINT              },
		{ Format::RG32_FLOAT,        VK_FORMAT_R32G32_SFLOAT            },
		{ Format::RGB32_UINT,        VK_FORMAT_R32G32B32_UINT           },
		{ Format::RGB32_SINT,        VK_FORMAT_R32G32B32_SINT           },
		{ Format::RGB32_FLOAT,       VK_FORMAT_R32G32B32_SFLOAT         },
		{ Format::RGBA32_UINT,       VK_FORMAT_R32G32B32A32_UINT        },
		{ Format::RGBA32_SINT,       VK_FORMAT_R32G32B32A32_SINT        },
		{ Format::RGBA32_FLOAT,      VK_FORMAT_R32G32B32A32_SFLOAT      },
		{ Format::D16,               VK_FORMAT_D16_UNORM                },
		{ Format::D24S8,             VK_FORMAT_D24_UNORM_S8_UINT        },
		{ Format::X24G8_UINT,        VK_FORMAT_D24_UNORM_S8_UINT        },
		{ Format::D32,               VK_FORMAT_D32_SFLOAT               },
		{ Format::D32S8,             VK_FORMAT_D32_SFLOAT_S8_UINT       },
		{ Format::X32G8_UINT,        VK_FORMAT_D32_SFLOAT_S8_UINT       },
		{ Format::BC1_UNORM,         VK_FORMAT_BC1_RGBA_UNORM_BLOCK     },
		{ Format::BC1_UNORM_SRGB,    VK_FORMAT_BC1_RGBA_SRGB_BLOCK      },
		{ Format::BC2_UNORM,         VK_FORMAT_BC2_UNORM_BLOCK          },
		{ Format::BC2_UNORM_SRGB,    VK_FORMAT_BC2_SRGB_BLOCK           },
		{ Format::BC3_UNORM,         VK_FORMAT_BC3_UNORM_BLOCK          },
		{ Format::BC3_UNORM_SRGB,    VK_FORMAT_BC3_SRGB_BLOCK           },
		{ Format::BC4_UNORM,         VK_FORMAT_BC4_UNORM_BLOCK          },
		{ Format::BC4_SNORM,         VK_FORMAT_BC4_SNORM_BLOCK          },
		{ Format::BC5_UNORM,         VK_FORMAT_BC5_UNORM_BLOCK          },
		{ Format::BC5_SNORM,         VK_FORMAT_BC5_SNORM_BLOCK          },
		{ Format::BC6H_UFLOAT,       VK_FORMAT_BC6H_UFLOAT_BLOCK        },
		{ Format::BC6H_SFLOAT,       VK_FORMAT_BC6H_SFLOAT_BLOCK        },
		{ Format::BC7_UNORM,         VK_FORMAT_BC7_UNORM_BLOCK          },
		{ Format::BC7_UNORM_SRGB,    VK_FORMAT_BC7_SRGB_BLOCK           },

	} };

	nvrhi::Format VulkanFormatFromVkFormat(VkFormat vkFormat)
	{
		for (size_t i = 0; i < c_FormatMap.size(); i++)
		{
			if (c_FormatMap[i].vkFormat == vkFormat)
				return c_FormatMap[i].rhiFormat;
		}

		return nvrhi::Format::UNKNOWN; // 找不到的話
	}
}
