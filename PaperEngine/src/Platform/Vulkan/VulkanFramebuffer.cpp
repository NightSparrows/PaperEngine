#include "VulkanFramebuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpec& spec) :
		m_renderPass(spec.renderPass), m_width(spec.width), m_height(spec.height)
	{
		std::vector<VkImageView> attachments(spec.attachments.size());
		m_attachments.reserve(spec.attachments.size());
		for (uint32_t i = 0; i < spec.attachments.size(); i++) {
			const auto& attData = spec.attachments[i];

			auto& attInfo = m_attachments.emplace_back();

			auto vkTexture = std::static_pointer_cast<VulkanTexture>(attData.texture);
			attachments[i] = vkTexture->get_image_view();

			if (vkTexture->get_format() == VulkanContext::GetDepthFormat()) {
				attInfo.clearValue.depthStencil = { .depth = attData.clearDepth, .stencil = attData.clearStencil };
			}
			else {
				attInfo.clearValue.color.float32[0] = attData.clearColor.r;
				attInfo.clearValue.color.float32[1] = attData.clearColor.g;
				attInfo.clearValue.color.float32[2] = attData.clearColor.b;
				attInfo.clearValue.color.float32[3] = attData.clearColor.a;
			}
			attInfo.texture = vkTexture;
		}
		VkFramebufferCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = spec.renderPass,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = spec.width,
			.height = spec.height,
			.layers = 1
		};

		CHECK_VK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice(), &createInfo, nullptr, &m_handle));
		PE_CORE_TRACE("Framebuffer created. {}", (void*)m_handle);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		vkDestroyFramebuffer(VulkanContext::GetDevice(), m_handle, nullptr);
		PE_CORE_TRACE("Framebuffer destroyed. {}", (void*)m_handle);
	}

}
