#include <span>

#include "VulkanCommandBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "VulkanBuffer.h"

namespace PaperEngine {

	VulkanCommandBuffer::VulkanCommandBuffer(const CommandBufferSpec& spec) :
		m_isPrimary(spec.isPrimary)
	{
		// TODO: get the current command pool in flight
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
	}

	void VulkanCommandBuffer::open()
	{
		auto getCmd = VulkanContext::GetCommandBufferManager()->allocate(m_isPrimary);
		PE_CORE_ASSERT(getCmd.buffer != VK_NULL_HANDLE, "Failed to allocated command buffer from manager");

		m_handle = getCmd.buffer;
		m_pool = getCmd.pool;

		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};

		vkBeginCommandBuffer(m_handle, &beginInfo);
	}

	void VulkanCommandBuffer::close()
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot close.");
		vkEndCommandBuffer(m_handle);
	}

	void VulkanCommandBuffer::setTextureState(TextureHandle texture, TextureState state)
	{
		Ref<VulkanTexture> vkTexture = std::static_pointer_cast<VulkanTexture>(texture);

		VkPipelineStageFlags srcStageMask;
		VkPipelineStageFlags dstStageMask;
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = vkTexture->get_image();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		switch (state)
		{
		case TextureState::TransferDst:
			srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;			// when to start
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// let it ready to transfer
			//barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// usually undefined, special case I dont care now
			barrier.oldLayout = vkTexture->m_currentLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;	
			vkTexture->m_currentLayout = barrier.newLayout;

			// TODO: unlike present is almost static in subresources, setting subresourceRange in future
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			break;
		case TextureState::TransferSrc:
			srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;			// when to start
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;		// let it ready to be read for transfer
			//barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;				// usually undefined, special case I dont care now
			barrier.oldLayout = vkTexture->m_currentLayout;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			vkTexture->m_currentLayout = barrier.newLayout;

			// TODO: unlike present is almost static in subresources, setting subresourceRange in future
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			break;
		case TextureState::Present:
			srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;		// wait this stage to be finish (must after this stage) (where to wait for previous works to finish)
			dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// wait after this stage (last stage) (block the next work from starting)
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = 0;
			barrier.oldLayout = vkTexture->m_currentLayout;		// TODO: track in texture handle
			vkTexture->m_currentLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// update the layout
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			break;
		default:
			PE_CORE_ASSERT(false, "Unknown texture state");
			return;
		}
		vkCmdPipelineBarrier(m_handle, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		m_textures.push_back(texture);
	}

	void VulkanCommandBuffer::writeBuffer(BufferHandle buffer, const void* data, size_t size, size_t offset)
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot write buffer.");
		m_stagingBuffer = VulkanContext::GetCommandBufferManager()->getStagingBuffer();

		PE_CORE_ASSERT(m_stagingBuffer, "No staging buffer!");
		m_stagingBuffer->stageBuffer(m_handle, data, std::static_pointer_cast<VulkanBuffer>(buffer)->get_handle(), (VkDeviceSize)size, offset);
		m_buffers.push_back(buffer);
	}

	void VulkanCommandBuffer::bindDescriptorSet(uint32_t setSlot, DescriptorSetHandle set, BindPoint bindPoint)
	{
		this->bindDescriptorSets(0, 1, &set, bindPoint);
	}

	void VulkanCommandBuffer::bindDescriptorSets(uint32_t firstSet, uint32_t setCount, DescriptorSetHandle* sets, BindPoint bindPoint)
	{
		PE_CORE_ASSERT(m_currentGraphicsPipeline, "Graphics Pipeline is not bound, can not bind descriptor set.");
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot bind descriptor set.");
		std::vector<VkDescriptorSet> rawSets;
		rawSets.reserve(setCount);
		for (uint32_t i = 0; i < setCount; i++) {
			rawSets.push_back(std::static_pointer_cast<VulkanDescriptorSet>(sets[i])->get_handle());
			m_descriptorSets.push_back(sets[i]);
		}
		vkCmdBindDescriptorSets(m_handle, ConvertBindPoint(bindPoint), m_currentGraphicsPipeline->get_layout(), firstSet, static_cast<uint32_t>(rawSets.size()), rawSets.data(), 0, nullptr);
	}

	void VulkanCommandBuffer::bindGraphicsPipeline(GraphicsPipelineHandle graphicsPipeline)
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot bind graphics pipeline.");
		auto vGPipeline = std::static_pointer_cast<VulkanGraphicsPipeline>(graphicsPipeline);
		vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, vGPipeline->get_handle());
		m_graphicsPipeline.push_back(vGPipeline);
		m_currentGraphicsPipeline = vGPipeline;
	}

	void VulkanCommandBuffer::bindIndexBuffer(BufferHandle buffer, uint32_t offset)
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot bind index buffer.");
		vkCmdBindIndexBuffer(m_handle, std::static_pointer_cast<VulkanBuffer>(buffer)->get_handle(), offset, VK_INDEX_TYPE_UINT32);
		m_buffers.push_back(buffer);
	}

	void VulkanCommandBuffer::drawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t instanceCount, uint32_t vertexOffset, uint32_t firstInstance)
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot draw.");
		vkCmdDrawIndexed(m_handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void VulkanCommandBuffer::beginFramebuffer(VulkanFramebufferHandle framebuffer)
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot begin framebuffer.");

		std::vector<VkClearValue> clearValues;
		clearValues.reserve(framebuffer->get_attachments().size());
		for (const auto& attInfo : framebuffer->get_attachments()) {
			clearValues.push_back(attInfo.clearValue);
		}

		VkRenderPassBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = framebuffer->get_render_pass(),
			.framebuffer = framebuffer->get_handle(),
			.renderArea = {
				.offset = {0, 0},
				.extent = {
					.width = framebuffer->get_width(),
					.height = framebuffer->get_height()
				}
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};
		vkCmdBeginRenderPass(m_handle, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		m_framebuffers.push_back(framebuffer);	// hold it
		m_currentFramebuffer = framebuffer;
	}

	void VulkanCommandBuffer::endFramebuffer()
	{
		PE_CORE_ASSERT(m_handle != VK_NULL_HANDLE, "Command buffer is not open, cannot end framebuffer.");
		vkCmdEndRenderPass(m_handle);
		// change the framebuffer attachment state
		// 基本上不是depthStenilAttachment就一定是colorAttachment
		for (auto& attInfo : m_currentFramebuffer->get_attachments()) {
			if (attInfo.texture->get_format() == VulkanContext::GetDepthFormat()) {
				attInfo.texture->m_currentLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}
			else {
				attInfo.texture->m_currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}
		m_currentFramebuffer = nullptr;
	}

	VkPipelineBindPoint VulkanCommandBuffer::ConvertBindPoint(BindPoint bindPoint)
	{
		switch (bindPoint)
		{
		case PaperEngine::Graphics:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PaperEngine::Compute:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		default:
			PE_CORE_ASSERT(false, "Wired bind point");
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		}
	}

	void VulkanCommandBuffer::releaseObject()
	{
		m_graphicsPipeline.clear();
		m_descriptorSets.clear();
		m_framebuffers.clear();
		m_buffers.clear();
		m_textures.clear();
		if (m_stagingBuffer) {
			m_stagingBuffer->release(m_handle);
			m_stagingBuffer = nullptr;
		}
		m_handle = VK_NULL_HANDLE;
	}

}
