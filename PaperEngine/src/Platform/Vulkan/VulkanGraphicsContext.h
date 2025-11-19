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

class NVMessageCallback : public nvrhi::IMessageCallback {
public:
	void message(nvrhi::MessageSeverity severity, const char* messageText) override {
		switch (severity)
		{
		case nvrhi::MessageSeverity::Info:
			PE_CORE_INFO("[NVRHI] {}", messageText);
			break;
		case nvrhi::MessageSeverity::Warning:
			PE_CORE_WARN("[NVRHI] {}", messageText);
			break;
		case nvrhi::MessageSeverity::Error:
			PE_CORE_ERROR("[NVRHI] {}", messageText);
			//PE_DEBUGBREAK();
			break;
		case nvrhi::MessageSeverity::Fatal:
			PE_CORE_CRITICAL("[NVRHI] {}", messageText);
			PE_DEBUGBREAK();
			break;
		default:
			break;
		}
	}
};

namespace PaperEngine {
	struct VulkanInstance {

		vkb::Instance vkbInstance;
		VkSurfaceKHR surface = VK_NULL_HANDLE;
		vkb::Device vkbDevice;
		vkb::PhysicalDevice physicalDevice;
		vkb::Swapchain vkbSwapchain;

		VkQueue graphicsQueue = VK_NULL_HANDLE;
		uint32_t graphicsQueueIndex = UINT32_MAX;

		VkQueue computeQueue = VK_NULL_HANDLE;
		uint32_t computeQueueIndex = UINT32_MAX;

		VkQueue transferQueue = VK_NULL_HANDLE;
		uint32_t transferQueueIndex = UINT32_MAX;

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
		struct SwapchainFramebuffer {
			nvrhi::TextureHandle depthTexture;
			nvrhi::FramebufferHandle framebuffer;
		};
		std::vector<SwapchainFramebuffer> framebuffers;

		nvrhi::Format depthFormat = nvrhi::Format::UNKNOWN;

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

		void setOnBackBufferResizingCallback(const std::function<void()>& callback) override;

		void setOnBackBufferResizedCallback(const std::function<void()>& callback) override;

		nvrhi::IDevice* getNVRhiDevice() const override;

		nvrhi::FramebufferHandle getCurrentFramebuffer() override;

		nvrhi::Format getSupportedDepthFormat() override;

	private:
		bool createSwapchain();

		void createFramebuffers();

	private:
		VulkanInstance m_instance;

		NVMessageCallback m_NVMsgCallback;

		GLFWWindow* m_window;

		std::function<void()> m_onBackBufferResizingCallback;
		std::function<void()> m_onBackBufferResizedCallback;

		bool m_resizeRequested = false;
	};
}

namespace nvrhi
{
	nvrhi::Format VulkanFormatFromVkFormat(VkFormat format);
}