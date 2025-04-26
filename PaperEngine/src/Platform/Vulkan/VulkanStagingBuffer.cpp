#include "VulkanStagingBuffer.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanStagingBuffer::VulkanStagingBuffer(VkDeviceSize chunkSize) :
		m_chunkSize(chunkSize)
	{

	}

	VulkanStagingBuffer::~VulkanStagingBuffer()
	{
		for (const auto& chunk : m_chunks) {
			vmaDestroyBuffer(VulkanContext::GetAllocator(), chunk.buffer, chunk.allocation);
			PE_CORE_TRACE("Staging chunk destroyed. {}", (void*)chunk.buffer);
		}
	}

	void VulkanStagingBuffer::stageBuffer(VkCommandBuffer cmd, const void* data, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset)
	{
		std::vector<uint32_t>& useChunk = m_useMap[cmd];
		for (uint32_t i = 0; i < m_chunks.size(); i++) {
			if (size == 0)
				break;
			auto& chunk = m_chunks[i];

			if (chunk.isInUse)
				continue;

			//PE_CORE_TRACE("staging chunk use. {}", (void*)chunk.buffer);

			VkDeviceSize insertSize = size > m_chunkSize ? m_chunkSize : size;

			doCopyBuffer(cmd, i, data, dstBuffer, insertSize, offset);

			size -= insertSize;
			offset += insertSize;
		}

		while (size > 0) {
			auto& chunk = create_chunk();

			VkDeviceSize insertSize = size > m_chunkSize ? m_chunkSize : size;

			doCopyBuffer(cmd, (uint32_t)m_chunks.size() - 1, data, dstBuffer, insertSize, offset);

			size -= insertSize;
			offset += insertSize;
		}
	}

	void VulkanStagingBuffer::doCopyBuffer(VkCommandBuffer cmd, uint32_t chunkIndex, const void* data, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset)
	{
		auto& chunk = m_chunks[chunkIndex];


		VkBufferCopy copy = {
			.srcOffset = 0,
			.dstOffset = offset,
			.size = size
		};

		void* stageBufPtr = nullptr;
		CHECK_VK_RESULT(vmaMapMemory(VulkanContext::GetAllocator(), chunk.allocation, &stageBufPtr));
		memcpy_s(stageBufPtr, size, (const char*)data + offset, size);
		vmaUnmapMemory(VulkanContext::GetAllocator(), chunk.allocation);

		vkCmdCopyBuffer(cmd, chunk.buffer, dstBuffer, 1, &copy);

		chunk.isInUse = true;
		//PE_CORE_TRACE("staging chunk use. {}", (void*)chunk.buffer);
		m_useMap[cmd].push_back(chunkIndex);
	}

	void VulkanStagingBuffer::stageTexture(VkCommandBuffer cmd, const void* data, VulkanTextureHandle texture, VkOffset2D offset, VkExtent2D size)
	{
		std::vector<uint32_t>& useChunk = m_useMap[cmd];

		// TODO: get format to determine pixel data
		uint32_t pixelSize = 4;

		uint32_t totalSize = size.width * size.height * pixelSize;

		// load by row major
		uint32_t chunkHeight = (uint32_t)m_chunkSize / size.width;

		uint32_t dataUploadOffset = 0;

		for (uint32_t i = 0; i < m_chunks.size(); i++) {
			if (totalSize == 0)
				break;
			auto& chunk = m_chunks[i];

			if (chunk.isInUse)
				continue;

			chunk.isInUse = true;
			//PE_CORE_TRACE("staging chunk use. {}", (void*)chunk.buffer);

			uint32_t insertSize = totalSize > static_cast<uint32_t>(m_chunkSize) ? static_cast<uint32_t>(m_chunkSize) : totalSize;
			uint32_t insertHeight = insertSize / size.width / pixelSize;

			uint32_t insertOffsetHeight = dataUploadOffset / size.width / pixelSize;

			VkBufferImageCopy copy = {
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.imageOffset = {
					.x = offset.x,
					.y = offset.y + static_cast<int32_t>(insertOffsetHeight),
					.z = 0
				},
				.imageExtent = {
					.width = size.width,
					.height = insertHeight,
					.depth = 1,
				}
			};


			void* stageBufPtr = nullptr;
			CHECK_VK_RESULT(vmaMapMemory(VulkanContext::GetAllocator(), chunk.allocation, &stageBufPtr));
			memcpy_s(stageBufPtr, insertSize, (const char*)data + dataUploadOffset, insertSize);
			vmaUnmapMemory(VulkanContext::GetAllocator(), chunk.allocation);

			vkCmdCopyBufferToImage(cmd, chunk.buffer, texture->get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
			totalSize -= insertSize;
			dataUploadOffset += insertSize;
			useChunk.push_back(i);
		}

		while (totalSize > 0) {
			auto& chunk = create_chunk();

			chunk.isInUse = true;
			//PE_CORE_TRACE("staging chunk use. {}", (void*)chunk.buffer);
			uint32_t insertSize = totalSize > static_cast<uint32_t>(m_chunkSize) ? static_cast<uint32_t>(m_chunkSize) : totalSize;
			uint32_t insertHeight = insertSize / size.width / pixelSize;

			uint32_t insertOffsetHeight = dataUploadOffset / size.width / pixelSize;

			VkBufferImageCopy copy = {
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1
				},
				.imageOffset = {
					.x = offset.x,
					.y = offset.y + static_cast<int32_t>(insertOffsetHeight),
					.z = 0
				},
				.imageExtent = {
					.width = size.width,
					.height = insertHeight,
					.depth = 1,
				}
			};


			void* stageBufPtr = nullptr;
			CHECK_VK_RESULT(vmaMapMemory(VulkanContext::GetAllocator(), chunk.allocation, &stageBufPtr));
			memcpy_s(stageBufPtr, insertSize, (const char*)data + dataUploadOffset, insertSize);
			vmaUnmapMemory(VulkanContext::GetAllocator(), chunk.allocation);

			vkCmdCopyBufferToImage(cmd, chunk.buffer, texture->get_image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
			totalSize -= insertSize;
			dataUploadOffset += insertSize;
			useChunk.push_back((uint32_t)m_chunks.size() - 1);
		}
	}

	void VulkanStagingBuffer::release(VkCommandBuffer cmd)
	{
		auto it = m_useMap.find(cmd);
		if (it == m_useMap.end()) {
			PE_CORE_ERROR("Wired releasing staging buffer using command buffer {} not found", (void*)cmd);
			return;
		}

		for (const auto& index : it->second) {
			PE_CORE_ASSERT(index < m_chunks.size(), "Index out of bound.");
			m_chunks[index].isInUse = false;
			//PE_CORE_TRACE("staging chunk released. {}", (void*)m_chunks[index].buffer);
		}

		m_useMap.erase(it);
	}

	VulkanStagingBuffer::BufferChunk& VulkanStagingBuffer::create_chunk()
	{
		auto& chunk = m_chunks.emplace_back();
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = m_chunkSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,				// used for transfer source
		};
		VmaAllocationCreateInfo allocInfo = {
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO
		};
		CHECK_VK_RESULT(vmaCreateBuffer(VulkanContext::GetAllocator(), &bufferInfo, &allocInfo, &chunk.buffer, &chunk.allocation, nullptr));
		PE_CORE_TRACE("Staging chunk created. {}", (void*)chunk.buffer);

		return chunk;
	}

}
