#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <PaperEngine/renderer/Buffer.h>

namespace PaperEngine {

	class VulkanBuffer : public Buffer {
	public:
		VulkanBuffer(const BufferSpecification& spec);
		~VulkanBuffer();

		VkBuffer get_handle() const { return m_buffer; }

		VkDeviceSize get_size() const { return m_size; }

	protected:
		VkBuffer m_buffer;
		VmaAllocation m_allocation;

		VkDeviceSize m_size;
	};

}
