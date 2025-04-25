#include "VulkanFramebuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpec& spec) :
		m_renderPass(spec.renderPass), m_width (spec.width), m_height(spec.height)
	{
		std::vector<VkImageView> attachments(spec.attachments.size());
		m_attachments.reserve(spec.attachments.size());
		for (uint32_t i = 0; i < spec.attachments.size(); i++) {
			auto& attInfo = m_attachments.emplace_back();

			auto vkTexture = std::static_pointer_cast<VulkanTexture>(spec.attachments[i]);
			attachments[i] = vkTexture->get_image_view();
			
			attInfo.clearValue = spec.clearValues[i];
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
