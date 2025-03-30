#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <PaperEngine/renderer/GraphicsContext.h>

#include <Platform/Vulkan/VulkanPhysicalDeviceSelector.h>

namespace PaperEngine {

	class VulkanContext : public GraphicsContext {
	public:

		VulkanContext(GLFWwindow* handle);
		~VulkanContext();

		void init() override;
		void swapBuffers() override;

	private:
		GLFWwindow* m_windowHandle{ nullptr };

		VkInstance m_instance{ VK_NULL_HANDLE };
#ifdef PE_DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };
#endif
		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

		VulkanPhysicalDeviceSelector m_phys_selector;
		int m_queue_family = -1;						// the queue family going to present

		VkDevice m_device{ VK_NULL_HANDLE };
	};

}
