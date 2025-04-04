
#include <array>

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/core/Application.h>
#include <Platform/Vulkan/VulkanContext.h>
#include <Platform/Vulkan/VulkanUtils.h>

#include "VulkanRenderer.h"


namespace PaperEngine {


	void PaperEngine::VulkanRenderer::init()
	{
		std::array<VkAttachmentDescription, 1> attachments;
		attachments[0] = {
			.format = VulkanContext::GetSwapchainSurfaceFormat().format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		};

		std::array<VkAttachmentReference, 1> subpass0ColorReferences;
		subpass0ColorReferences[0] = {
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		std::array<VkSubpassDescription, 1> subpasses;
		subpasses[0] = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.inputAttachmentCount = 0,
			.pInputAttachments = nullptr,
			.colorAttachmentCount = static_cast<uint32_t>(subpass0ColorReferences.size()),
			.pColorAttachments = subpass0ColorReferences.data(),
			.pResolveAttachments = nullptr,
			.pDepthStencilAttachment = nullptr,
			.preserveAttachmentCount = 0,
			.pPreserveAttachments = nullptr
		};

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = 0;
		renderPassInfo.pDependencies = nullptr;

		CHECK_VK_RESULT(vkCreateRenderPass(VulkanContext::GetDevice(), &renderPassInfo, nullptr, &m_renderPass));
	
		PE_CORE_TRACE("[VULKAN] Render pass created.");
		this->createFramebuffers();
	}

	void VulkanRenderer::cleanUp()
	{
		for (size_t i = 0; i < m_framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(VulkanContext::GetDevice(), m_framebuffers[i], nullptr);
		}
		m_framebuffers.clear();
		vkDestroyRenderPass(VulkanContext::GetDevice(), m_renderPass, nullptr);
		m_renderPass = VK_NULL_HANDLE;
		PE_CORE_TRACE("[VULKAN] Render pass destroyed.");
	}

	void VulkanRenderer::begin_frame()
	{
		std::array<VkClearValue, 1> clearValues;
		clearValues[0] = {
			.color = { 1.0f, 0.0f, 0.0f, 1.0f }
		};

		VkRenderPassBeginInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = m_renderPass,
			.framebuffer = m_framebuffers[VulkanContext::GetCurrentImageIndex()],
			.renderArea = {
				.offset = { 0, 0 },
				.extent = {
					Application::Get().get_window().get_width(),
					Application::Get().get_window().get_height()
				}
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};

		vkCmdBeginRenderPass(VulkanContext::GetCurrentCmdBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderer::end_frame()
	{
		vkCmdEndRenderPass(VulkanContext::GetCurrentCmdBuffer());
	}

	void VulkanRenderer::createFramebuffers()
	{
		m_framebuffers.resize(VulkanContext::GetImageCount());
		for (size_t i = 0; i < m_framebuffers.size(); i++)
		{
			std::array<VkImageView, 1> attachments = {
				VulkanContext::GetSwapchainImageViews()[i]
			};
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = Application::Get().get_window().get_width();
			framebufferInfo.height = Application::Get().get_window().get_height();
			framebufferInfo.layers = 1;
			CHECK_VK_RESULT(vkCreateFramebuffer(VulkanContext::GetDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]));
		}
	}
}
