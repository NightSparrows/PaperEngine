#pragma once

#include <vulkan/vulkan.h>

#include <PaperEngine/core/Base.h>
#include "VulkanTexture.h"

namespace PaperEngine {

	struct FramebufferSpec {
		uint32_t width, height;
		std::vector<TextureHandle> attachments;
		VkRenderPass renderPass;
		std::vector<VkClearValue> clearValues;		// if the render pass determine that is clear in initial state, otherwise ignore it
	};

	class VulkanFramebuffer {
	public:
		struct AttachmentInfo {
			VkClearValue clearValue;
			VulkanTextureHandle texture;
		};

		VulkanFramebuffer(const FramebufferSpec& spec);
		~VulkanFramebuffer();

		VkFramebuffer get_handle() const { return m_handle; }

		VkRenderPass get_render_pass() const { return m_renderPass; }

		uint32_t get_width() const { return m_width; }

		uint32_t get_height() const { return m_height; }

		const std::vector<AttachmentInfo>& get_attachments() const { return m_attachments; }

	protected:
		VkFramebuffer m_handle{ VK_NULL_HANDLE };

		// not manage it, just who is the template of this framebuffer
		VkRenderPass m_renderPass{ VK_NULL_HANDLE };

		uint32_t m_width, m_height;

		std::vector<AttachmentInfo> m_attachments;
	};

	typedef Ref<VulkanFramebuffer> VulkanFramebufferHandle;
}
