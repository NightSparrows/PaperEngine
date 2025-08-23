#pragma once


#include <PaperEngine/graphics/GraphicsContext.h>
#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>

#include <VKBootstrap.h>

#include <PaperEngine/core/GLFWWindow.h>

#ifdef PE_DEBUG
#define CHECK_VK_RESULT(x) \
				do \
				{	\
					VkResult err = x;	\
					if (err)	\
					{				\
						PE_CORE_ERROR("Vulkan Error: {} in {}:{}", (uint32_t)err, __FILE__, __LINE__); \
						PE_DEBUGBREAK();	\
					}	\
				} while (0) 
#else
// just call it without checking it
#define CHECK_VK_RESULT(x) x
#endif

namespace PaperEngine {
	struct VulkanInstance {

		vkb::Instance vkbInstance;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		vkb::Device vkbDevice;
		vkb::PhysicalDevice physicalDevice;
		vkb::Swapchain vkbSwapchain;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueIndex = -1;

		VkQueue computeQueue = VK_NULL_HANDLE;
		uint32_t computeQueueIndex = -1;

		VkQueue transferQueue = VK_NULL_HANDLE;
		uint32_t transferQueueIndex = -1;

		nvrhi::DeviceHandle device;
#ifdef PE_DEBUG
		nvrhi::DeviceHandle validationDevice;
#endif // PE_DEBUG

		std::vector<nvrhi::TextureHandle> swapchainTextures;
		uint32_t swapchainIndex = 0;

		uint32_t imageAvailableFenceIndex = 0;
		VkFence currentImageAvailableFence = { VK_NULL_HANDLE };
		std::vector<VkFence> imageAvailableFences;


		/// <summary>
		/// swapchain被使用的framebuffer
		/// </summary>
		std::vector<nvrhi::FramebufferHandle> framebuffers;

	};

	class VulkanGraphicsContext : public GraphicsContext {
	public:
		VulkanGraphicsContext(Window* window);

		void init() override;

		void cleanUp() override;

		bool beginFrame() override;

		bool present() override;

		void waitForSwapchainImageAvailable() override;

		uint32_t getSwapchainCount() override;

		uint32_t getSwapchainIndex() const override;

		nvrhi::TextureHandle getCurrentSwapchainTexture() override;

		void submitFinalDrawCmd(nvrhi::CommandListHandle cmd) override;

		void setOnBackBufferResizingCallback(const std::function<void()>& callback) override;

		void setOnBackBufferResizedCallback(const std::function<void()>& callback) override;

		nvrhi::DeviceHandle getNVRhiDevice() const override;

		nvrhi::FramebufferHandle getCurrentFramebuffer() override;

	private:
		bool createSwapchain();

		void createFramebuffers();

	private:
		VulkanInstance m_instance;

		GLFWWindow* m_window;

		std::function<void()> m_onBackBufferResizingCallback;
		std::function<void()> m_onBackBufferResizedCallback;

		bool m_resizeRequested = false;

		std::vector<nvrhi::CommandListHandle> m_frameCommandLists;
	};
}
