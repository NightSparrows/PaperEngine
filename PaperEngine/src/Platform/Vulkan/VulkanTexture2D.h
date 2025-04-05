#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace PaperEngine {

	/// <summary>
	/// No material properties
	/// </summary>
	class VulkanTexture2D {
	public:
		VulkanTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);

		~VulkanTexture2D();

		void transitLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout, VkImageLayout oldLayout = VK_IMAGE_LAYOUT_MAX_ENUM);

		void record_copy_in_gpu(VkCommandBuffer cmd, VkBuffer srcBuffer, VkDeviceSize src_offset = 0);

		VkImage get_image() const { return m_image; }

		VkImageView get_image_view() const { return m_imageView; }

	private:

		uint32_t m_width, m_height;

		VkFormat m_format{ VK_FORMAT_UNDEFINED };

		// not correct at runtime
		VkImageLayout m_currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };

		VkImage m_image{ VK_NULL_HANDLE };
		VkImageView m_imageView{ VK_NULL_HANDLE };			// default 2d view for this image
		VmaAllocation m_allocation{ VK_NULL_HANDLE };
	};

}
