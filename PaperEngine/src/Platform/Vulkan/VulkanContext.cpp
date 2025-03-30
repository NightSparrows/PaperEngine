
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
		if (m_instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_instance, nullptr);
		}
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
	}

	void VulkanContext::swapBuffers()
	{
	}
}
