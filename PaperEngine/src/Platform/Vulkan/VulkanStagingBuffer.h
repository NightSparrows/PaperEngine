#pragma once

#include <unordered_map>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <PaperEngine/core/Base.h>
#include <Platform/Vulkan/VulkanTexture.h>

namespace PaperEngine {

	/// <summary>
	/// Not thread safe!!
	/// </summary>
	class VulkanStagingBuffer {
	public:
		VulkanStagingBuffer(VkDeviceSize chunkSize = 65536);
		~VulkanStagingBuffer();

		/// <summary>
		/// Store the data to staging buffer and ready to be upload
		/// </summary>
		/// <param name="cmd"></param>
		/// <param name="data"></param>
		/// <param name="dstBuffer"></param>
		/// <param name="size"></param>
		/// <param name="offset"></param>
		void stageBuffer(VkCommandBuffer cmd, const void* data, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset = 0);

		void stageTexture(VkCommandBuffer cmd, const void* data, VulkanTextureHandle texture, VkOffset2D offset, VkExtent2D size);

		void release(VkCommandBuffer cmd);

	protected:
		struct BufferChunk {
			VkBuffer buffer{ VK_NULL_HANDLE };
			VmaAllocation allocation{ nullptr };
			bool isInUse{ false };
		};

		void doCopyBuffer(VkCommandBuffer cmd, uint32_t chunkIndex, const void* data, VkBuffer dstBuffer, VkDeviceSize size, VkDeviceSize offset);

	protected:

		BufferChunk& create_chunk();

	protected:


		std::unordered_map<VkCommandBuffer, std::vector<uint32_t>> m_useMap;

		std::vector<BufferChunk> m_chunks;

		VkDeviceSize m_chunkSize;

	};

	typedef Ref<VulkanStagingBuffer> VulkanStagingBufferHandle;
}
