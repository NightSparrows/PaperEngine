#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {

	class VulkanTexture : public Texture {
	public:
		VulkanTexture(const TextureSpecification& spec);
		~VulkanTexture();

		/// <summary>
		/// Create texture handle for native image
		/// </summary>
		/// <param name="image"></param>
		/// <param name="spec"></param>
		VulkanTexture(VkImage image, const TextureSpecification& spec);

		uint32_t get_width() const override { return m_width; }
		uint32_t get_height() const override { return m_height; }

		VkImage get_image() const { return m_image; }

		VkImageView get_image_view() const { return m_view; }

		VkFormat get_format() const { return m_format; }

	protected:
		void create_image_view();

	public:

		VkImageLayout m_currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };


	protected:
		VkImage m_image{ VK_NULL_HANDLE };
		VmaAllocation m_allocation{ nullptr };
		VkImageView m_view{ VK_NULL_HANDLE };

		VkFormat m_format{ VK_FORMAT_UNDEFINED };
		VkImageType m_type{ VK_IMAGE_TYPE_MAX_ENUM };

		// whether the source (VkImage) is native or not
		bool m_isNative{ false };

		uint32_t m_width = 0, m_height = 0;
	};

	typedef Ref<VulkanTexture> VulkanTextureHandle;
}
