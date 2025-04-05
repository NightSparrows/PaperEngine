
#include <PaperEngine/core/Logger.h>

#include "VulkanRenderPass.h"

#include <Platform/Vulkan/VulkanContext.h>
#include <Platform/Vulkan/VulkanUtils.h>

namespace PaperEngine {



	VulkanRenderPass::VulkanRenderPass(const RenderPassCreateInfo& createInfo, uint32_t width, uint32_t height) :
		m_createInfo(createInfo)
	{
#pragma region attachment_Init
		std::vector<VkAttachmentDescription> attachments(createInfo.attachments.size());
		uint32_t depth_buffer_index = UINT32_MAX;

		for (uint32_t i = 0; i < attachments.size(); i++) {
			const auto& attachment_desc = createInfo.attachments[i];
			auto& attachment = attachments[i];
			switch (attachment_desc.format)
			{
			case ImageFormat::DepthBuffer:
				attachment.format = VulkanContext::GetPhysicalDeviceInfo().depth_format;
				depth_buffer_index = i;
				break;
			case ImageFormat::Present:
				attachment.format = VulkanContext::GetSwapchainSurfaceFormat().format;
				break;
			case ImageFormat::DepthFormat:
				attachment.format = VulkanContext::GetPhysicalDeviceInfo().depth_format;
				break;
			case ImageFormat::SwapchainFormat:
				attachment.format = VulkanContext::GetSwapchainSurfaceFormat().format;
				break;
			default:
				PE_CORE_ASSERT(false, "Unknown format");
				break;
			}
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			switch (attachment_desc.loadOp)
			{
			case LoadOp::Clear:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				break;
			case LoadOp::DontCare:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			case LoadOp::Load:
				attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			default:
				PE_CORE_ASSERT(false, "Unknown laodOp");
				break;
			}
			switch (attachment_desc.storeOp)
			{
			case StoreOp::DontCare:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			case StoreOp::Store:
				attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			default:
				PE_CORE_ASSERT(false, "Unknown storeOp");
				break;
			}
			auto image_layout_to_vk_image_layout = [](ImageLayout layout) {
				switch (layout)
				{
				case PaperEngine::ImageLayout::Undefined:
					return VK_IMAGE_LAYOUT_UNDEFINED;
				case PaperEngine::ImageLayout::ColorAttachment:
					return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				case PaperEngine::ImageLayout::PresentSrc:
					return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				case PaperEngine::ImageLayout::DepthAttachment:
					return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				case PaperEngine::ImageLayout::ShaderReadOnly:
					return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				default:
					PE_CORE_ASSERT(false, "Unknown image layout");
					return VK_IMAGE_LAYOUT_MAX_ENUM;
				}
				};
			attachment.initialLayout = image_layout_to_vk_image_layout(attachment_desc.initLayout);
			attachment.finalLayout = image_layout_to_vk_image_layout(attachment_desc.finalLayout);
		}
#pragma endregion attachment_Init

		std::vector<VkAttachmentReference> input_attachment_ref(createInfo.input_attachments.size());
		for (uint32_t i = 0; i < input_attachment_ref.size(); i++) {
			auto& att_ref = input_attachment_ref[i];
			att_ref.attachment = createInfo.input_attachments[i];
			PE_CORE_ASSERT(createInfo.input_attachments[i] < attachments.size(), "input attachment index is greater than attachments size");
			switch (attachments[createInfo.input_attachments[i]].finalLayout)
			{
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				att_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				att_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}
		}

		std::vector<VkAttachmentReference> output_attachment_ref(createInfo.output_attachments.size());
		for (uint32_t i = 0; i < output_attachment_ref.size(); i++) {
			auto& att_ref = output_attachment_ref[i];
			att_ref.attachment = createInfo.output_attachments[i];
			PE_CORE_ASSERT(createInfo.output_attachments[i] < attachments.size(), "input attachment index is greater than attachments size");
			switch (attachments[createInfo.output_attachments[i]].finalLayout)
			{
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				att_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				att_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				break;
			default:
				break;
			}
		}

		VkSubpassDescription subpass_desc = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = static_cast<uint32_t>(input_attachment_ref.size()),
			.pInputAttachments = input_attachment_ref.data(),
			.colorAttachmentCount = static_cast<uint32_t>(output_attachment_ref.size()),
			.pColorAttachments = output_attachment_ref.data(),
		};
		VkAttachmentReference depth_ref;
		if (depth_buffer_index != UINT32_MAX) {
			depth_ref.attachment = depth_buffer_index;
			depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpass_desc.pDepthStencilAttachment = &depth_ref;
		}

		VkRenderPassCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = static_cast<uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass_desc,
		};

		CHECK_VK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice(), &create_info, nullptr, &m_renderPass));

		recreateFramebuffers(width, height);
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		for (const auto& framebufferInfo : m_framebuffers) {
			vkDestroyFramebuffer(VulkanContext::GetDevice(), framebufferInfo.handle, nullptr);
		}
		m_framebuffers.clear();
		if (m_renderPass) {
			vkDestroyRenderPass(VulkanContext::GetDevice(), m_renderPass, nullptr);
			m_renderPass = VK_NULL_HANDLE;
		}
	}

	void VulkanRenderPass::recreateFramebuffers(uint32_t width, uint32_t height)
	{
		for (const auto& framebufferInfo : m_framebuffers) {
			vkDestroyFramebuffer(VulkanContext::GetDevice(), framebufferInfo.handle, nullptr);
		}
		m_framebuffers.clear();

		for (uint32_t i = 0; i < VulkanContext::GetImageCount(); i++) {
			auto& framebufferInfo = m_framebuffers.emplace_back();
			std::vector<VkImageView> attachments(m_createInfo.attachments.size());
			for (uint32_t j = 0; j < m_createInfo.attachments.size(); j++) {
				const auto& att_info = m_createInfo.attachments[j];
				switch (att_info.format)
				{
				case ImageFormat::Present:
					attachments[j] = VulkanContext::GetSwapchainImageViews()[i];
					break;
				case PaperEngine::ImageFormat::DepthBuffer:
				case PaperEngine::ImageFormat::DepthFormat:
				{
					auto& texture = framebufferInfo.attachments.emplace_back(width, height, VulkanContext::GetPhysicalDeviceInfo().depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
					attachments[j] = texture.get_image_view();
				}
				break;
				case PaperEngine::ImageFormat::RGBA32:
				{
					auto& texture = framebufferInfo.attachments.emplace_back(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
					attachments[j] = texture.get_image_view();
				}
				break;
				case PaperEngine::ImageFormat::SwapchainFormat:
				{
					auto& texture = framebufferInfo.attachments.emplace_back(width, height, VulkanContext::GetSwapchainSurfaceFormat().format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
					attachments[j] = texture.get_image_view();
				}
				break;
				default:
					break;
				}
			}

			VkFramebufferCreateInfo create_info = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = m_renderPass,
				.attachmentCount = static_cast<uint32_t>(attachments.size()),
				.pAttachments = attachments.data(),
				.width = width,
				.height = height,
				.layers = 1
			};
			CHECK_VK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice(), &create_info, nullptr, &framebufferInfo.handle));
		}

	}

}
