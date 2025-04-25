#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanBuffer::VulkanBuffer(const BufferSpecification& spec) :
		m_size(spec.size)
	{
		VkBufferCreateInfo buffer_info = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = spec.size,
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};
		if (spec.isVertexBuffer) {
			buffer_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		}
		if (spec.isStorageBuffer) {
			buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (spec.isIndexBuffer) {
			buffer_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (spec.isUniformBuffer) {
			buffer_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		VmaAllocationCreateInfo alloc_info = {
			.usage = VMA_MEMORY_USAGE_GPU_ONLY
		};
		CHECK_VK_RESULT(vmaCreateBuffer(VulkanContext::GetAllocator(), &buffer_info, &alloc_info, &m_buffer, &m_allocation, nullptr));
	}

	VulkanBuffer::~VulkanBuffer()
	{
		vmaDestroyBuffer(VulkanContext::GetAllocator(), m_buffer, m_allocation);
	}

}
