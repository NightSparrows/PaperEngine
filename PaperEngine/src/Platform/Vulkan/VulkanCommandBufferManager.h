#pragma once

#include <span>
#include <thread>
#include <unordered_map>
#include <vector>
#include <mutex>

#include <vulkan/vulkan.h>

#include "VulkanCommandBuffer.h"
#include "VulkanStagingBuffer.h"

namespace PaperEngine {

	class VulkanCommandBufferManager {
	public:
		struct CommandBufferGetter {
			VkCommandBuffer buffer;
			VkCommandPool pool;
		};

		VulkanCommandBufferManager();
		~VulkanCommandBufferManager();

		/// <summary>
		/// </summary>
		/// <param name="primary">
		/// If it is false, will give secondary command buffer
		/// </param>
		/// <returns></returns>
		CommandBufferGetter allocate(bool primary = true);

		/// <summary>
		/// You must execute the command buffer all in same thread that allocate
		/// </summary>
		/// <param name="cmds"></param>
		void executeCommandBuffers(uint32_t count, Ref<CommandBuffer>* cmds);

		void executeCommandBuffer(Ref<CommandBuffer> cmd);

		VulkanStagingBufferHandle getStagingBuffer();

	protected:
		struct PoolInfo {
			VkCommandPool pool{ VK_NULL_HANDLE };
			std::mutex mutex;
			VkFence fence{ VK_NULL_HANDLE };							// the fence to wait after the execution
			uint32_t useCount{ 0 };
			std::thread::id threadId;
			std::unordered_map<VkCommandBuffer, bool> bufferUsing;

			VulkanStagingBufferHandle stagingBuffer;

			// Default constructor for PoolInfo
			PoolInfo() = default;

			PoolInfo(const PoolInfo&) = delete;
			PoolInfo& operator=(const PoolInfo&) = delete;

			// Allow move constructor and move assignment operator
			PoolInfo(PoolInfo&& other) noexcept
				: pool(other.pool), fence(other.fence), useCount(other.useCount),
				threadId(other.threadId), bufferUsing(std::move(other.bufferUsing)) {
				other.pool = VK_NULL_HANDLE;
				other.fence = VK_NULL_HANDLE;
			}

			PoolInfo& operator=(PoolInfo&& other) noexcept {
				if (this != &other) {
					pool = other.pool;
					fence = other.fence;
					useCount = other.useCount;
					threadId = other.threadId;
					bufferUsing = std::move(other.bufferUsing);

					other.pool = VK_NULL_HANDLE;
					other.fence = VK_NULL_HANDLE;
				}
				return *this;
			}
		};
		std::vector<PoolInfo> m_pools;
	};

	typedef Ref< VulkanCommandBufferManager> VulkanCommandBufferManagerHandle;
}
