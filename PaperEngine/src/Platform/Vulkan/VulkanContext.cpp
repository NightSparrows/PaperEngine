
#include <vector>

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanContext::VulkanContext(GLFWwindow* handle)
		: m_windowHandle(handle)
	{
	}

	VulkanContext::~VulkanContext()
	{
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
		}
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
		appInfo.apiVersion = VK_API_VERSION_1_3;

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

		m_phys_selector.init(m_instance, m_surface);
		m_queue_family = m_phys_selector.select_device(VK_QUEUE_GRAPHICS_BIT, true);

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
	}

	void VulkanContext::swapBuffers()
	{
	}
}
