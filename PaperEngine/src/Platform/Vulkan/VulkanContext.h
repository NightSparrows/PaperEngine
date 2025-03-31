#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <PaperEngine/renderer/GraphicsContext.h>

#include <Platform/Vulkan/VulkanPhysicalDeviceSelector.h>

#define PE_VULKAN_API_VERSION VK_API_VERSION_1_3

namespace PaperEngine {

	class VulkanContext : public GraphicsContext {
	public:

		VulkanContext(GLFWwindow* handle);
		~VulkanContext();

		void init() override;
		void swapBuffers() override;

		void beginFrame() override;
		void endFrame() override;

	private:
		void create_swapchain();

		/// <summary>
		/// Create the command pool for primary command buffer
		/// </summary>
		void create_cmd_pool();

	protected:

		// the instance of the context, only one context in this application
		static VulkanContext* s_instance;

	private:
		GLFWwindow* m_windowHandle{ nullptr };

		VkInstance m_instance{ VK_NULL_HANDLE };
#ifdef PE_DEBUG
		VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };
#endif
		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

		VulkanPhysicalDeviceSelector m_phys_selector;
		uint32_t m_queue_family = -1;						// the queue family going to present

		VkDevice m_device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };

		// the swapchain images
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;

		// primary command buffer for each swapchain image
		VkCommandPool m_cmdPool{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> m_cmdBuffers;

		VkQueue m_presentQueue{ VK_NULL_HANDLE };			// the queue for presentating
		VkSemaphore m_imageAvailableSem{ VK_NULL_HANDLE };	// signaled when an image is acquired
		VkSemaphore m_renderFinishedSem{ VK_NULL_HANDLE };	// signaled when rendering is complete
		VkFence m_inFlightFence{ VK_NULL_HANDLE };			// CPU-GPU synchronization

		uint32_t m_current_image_index{ UINT32_MAX };
	};

}
