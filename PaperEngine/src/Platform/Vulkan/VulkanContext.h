#pragma once

#include <queue>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <PaperEngine/renderer/GraphicsContext.h>

#define PE_VULKAN_API_VERSION VK_API_VERSION_1_3

#include <vk_mem_alloc.h>

#include <VkBootstrap.h>

#include <PaperEngine/core/Logger.h>

#include "VulkanDescriptorSetManager.h"
#include "VulkanCommandBufferManager.h"

namespace PaperEngine {

	class VulkanContext : public GraphicsContext {
	public:

		VulkanContext(GLFWwindow* handle);
		~VulkanContext();

		void init() override;


		GraphicsAPI getGraphicsAPI() override;


		void cleanUp() override;

		bool beginFrame() override;
		void endFrame() override;

		void executeCommandBuffer(CommandBufferHandle cmd) override;

		void executeCommandBuffers(uint32_t count, CommandBufferHandle* cmd) override;

		uint32_t get_swapchain_image_count() override;

		uint32_t get_current_swapchain_index() override;

		TextureHandle get_swapchain_texture(uint32_t swapchainIndex) override;

	public:

		static VkDevice GetDevice();

		static VkInstance GetInstance();

		static VmaAllocator GetAllocator();

		static uint32_t GetImageCount();

		static uint32_t GetCurrentImageIndex();

		static const vkb::PhysicalDevice& GetPhysicalDevice() { return s_instance->m_phys_device; }

		static const vkb::Swapchain& GetSwapchain() { return s_instance->m_swapchain; }

		static VkCommandPool GetCommandPool() { return s_instance->m_command_pool; }

		static VulkanDescriptorSetManagerHandle GetDescriptorSetManager() { return s_instance->m_setManager; }

		/// <summary>
		/// get the depth format the selected gpu supported
		/// </summary>
		/// <returns></returns>
		static VkFormat GetDepthFormat() { return s_instance->m_depthFormat; }

		static uint32_t GetGraphicsQueueIndex() { return s_instance->m_graphics_queue_index; }

		static VkQueue GetGraphicsQueue() { return s_instance->m_graphics_queue; }

		static VulkanCommandBufferManagerHandle GetCommandBufferManager() { return s_instance->m_cmdManager; }

		static VulkanTextureHandle GetSwapchainTexture(uint32_t index) { return std::static_pointer_cast<VulkanTexture>(s_instance->m_swapchain_textures[index]); }

	private:
		bool create_swapchain();

		void present();

	protected:

		// the instance of the context, only one context in this application
		static VulkanContext* s_instance;

	private:
		GLFWwindow* m_windowHandle{ nullptr };

		vkb::Instance m_instance;
		vkb::PhysicalDevice m_phys_device;
		VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
		vkb::Device m_device;
		VkQueue m_present_queue{ VK_NULL_HANDLE };
		VkQueue m_graphics_queue{ VK_NULL_HANDLE };
		uint32_t m_graphics_queue_index = 0;
		vkb::Swapchain m_swapchain;
		bool m_resizeRequested{ false };
		std::vector<TextureHandle> m_swapchain_textures;

		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

		std::vector<VkSemaphore> m_image_available_sems;
		uint32_t m_image_ava_sem_index{ 0 };
		std::vector<VkFence> m_frames_in_flight_fences;
		uint32_t m_frames_in_flight_index{ 0 };
		std::vector<VkSemaphore> m_render_finish_sems;
		uint32_t m_render_finish_sem_index{ 0 };

		VkCommandPool m_command_pool{ VK_NULL_HANDLE };

		/// <summary>
		/// The buffer need to submit in this frame
		/// </summary>
		std::vector<VkCommandBuffer> m_submit_buffers;

		// vulkan memory allocator
		VmaAllocator m_allocator{ VK_NULL_HANDLE };

		uint32_t m_current_image_index{ UINT32_MAX };

		// TODO: staging buffer uploader

		VulkanDescriptorSetManagerHandle m_setManager;

		VulkanCommandBufferManagerHandle m_cmdManager;
	};

}
