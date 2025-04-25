#include "VulkanCommandBufferManager.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {

	VulkanCommandBufferManager::VulkanCommandBufferManager()
	{
		uint32_t maxThread = std::thread::hardware_concurrency();
		m_pools.resize(maxThread);
		for (uint32_t i = 0; i < maxThread; i++) {
			auto& poolInfo = m_pools[i];
			
			VkCommandPoolCreateInfo createInfo = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = VulkanContext::GetGraphicsQueueIndex(),
			};
			CHECK_VK_RESULT(vkCreateCommandPool(VulkanContext::GetDevice(), &createInfo, nullptr, &poolInfo.pool));

			VkFenceCreateInfo fenceInfo = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			};

			CHECK_VK_RESULT(vkCreateFence(VulkanContext::GetDevice(), &fenceInfo, nullptr, &poolInfo.fence));

			// TODO: modify chunk size
			poolInfo.stagingBuffer = CreateRef<VulkanStagingBuffer>();

		}
		PE_CORE_TRACE("pool create for thread, count: {}", maxThread);
		PE_CORE_TRACE("[CommandBufferManager] initialized.");
	}

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		for (const auto& pool : m_pools) {
			vkDestroyCommandPool(VulkanContext::GetDevice(), pool.pool, nullptr);
			vkDestroyFence(VulkanContext::GetDevice(), pool.fence, nullptr);
		}
		PE_CORE_TRACE("[CommandBufferManager] clean up.");
	}

	VulkanCommandBufferManager::CommandBufferGetter VulkanCommandBufferManager::allocate(bool primary)
	{
		PoolInfo* usePoolInfo = nullptr;
		
		// first finding that there is current pool
		for (auto& poolInfo : m_pools) {
			std::lock_guard lock(poolInfo.mutex);
			if (poolInfo.threadId == std::this_thread::get_id() && poolInfo.useCount > 0) {
				usePoolInfo = &poolInfo;
				poolInfo.useCount++;
				break;
			}
		}

		if (!usePoolInfo) {
			for (auto& poolInfo : m_pools) {
				std::lock_guard lock(poolInfo.mutex);
				if (poolInfo.useCount == 0) {
					usePoolInfo = &poolInfo;
					poolInfo.useCount++;
					usePoolInfo->threadId = std::this_thread::get_id();
					break;
				}
			}
			if (!usePoolInfo) {
				PE_CORE_ERROR("No pool can be allocate!");
				PE_CORE_ASSERT(false, "No pool can be allocate!");
				return {VK_NULL_HANDLE, VK_NULL_HANDLE};
			}
		}
		// here is thread safe because same thread can not go here in parallel
		auto& bufferList = usePoolInfo->bufferUsing;
		for (auto& [cmdBuf, isUse] : bufferList) {
			if (!isUse) {
				isUse = true;
				return { cmdBuf, usePoolInfo->pool };
			}
		}

		// no free command buffer, allocate one
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = usePoolInfo->pool,
			.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = 1
		};
		VkCommandBuffer cmdBuf;
		CHECK_VK_RESULT(vkAllocateCommandBuffers(VulkanContext::GetDevice(), &allocInfo, &cmdBuf));
		bufferList[cmdBuf] = true;
		PE_CORE_TRACE("Command buffer created. {}", (void*)cmdBuf);

		return CommandBufferGetter{ cmdBuf, usePoolInfo->pool };
	}

	void VulkanCommandBufferManager::executeCommandBuffers(uint32_t count, Ref<CommandBuffer>* cmds)
	{
		if (count == 0)
			return;
		std::vector<VkCommandBuffer> cmdbuffers(count);
		VkCommandPool pool = std::static_pointer_cast<VulkanCommandBuffer>(cmds[0])->get_pool();
		for (uint32_t i = 0; i < count; i++) {
			PE_CORE_ASSERT(std::static_pointer_cast<VulkanCommandBuffer>(cmds[i])->get_pool() == pool, "Executing command buffers need to be in same pool.");
			cmdbuffers[i] = std::static_pointer_cast<VulkanCommandBuffer>(cmds[i])->get_handle();
		}
		VkSubmitInfo submit = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = static_cast<uint32_t>(cmdbuffers.size()),
			.pCommandBuffers = cmdbuffers.data()
		};
		PoolInfo* usePoolInfo = nullptr;
		for (auto& poolInfo : m_pools) {
			if (poolInfo.pool == pool) {
				usePoolInfo = &poolInfo;
				break;
			}
		}
		if (!usePoolInfo) {
			PE_CORE_ASSERT(false, "Wired pool is not using");
			return;
		}

		CHECK_VK_RESULT(vkQueueSubmit(VulkanContext::GetGraphicsQueue(), 1, &submit, usePoolInfo->fence));

		// wait fence
		CHECK_VK_RESULT(vkWaitForFences(VulkanContext::GetDevice(), 1, &usePoolInfo->fence, VK_TRUE, UINT64_MAX));
		CHECK_VK_RESULT(vkResetFences(VulkanContext::GetDevice(), 1, &usePoolInfo->fence));

		// release that
		for (uint32_t i = 0; i < count; i++) {
			auto cmd = std::static_pointer_cast<VulkanCommandBuffer>(cmds[i]);
			//PE_CORE_TRACE("Command buffer executed. {}", (void*)cmd->get_handle());
			CHECK_VK_RESULT(vkResetCommandBuffer(cmd->get_handle(), 0));
			cmd->releaseObject();
		}
		{
			std::lock_guard lock(usePoolInfo->mutex);
			usePoolInfo->useCount -= count;
			for (const auto& cmdBuf : cmdbuffers) {
				usePoolInfo->bufferUsing[cmdBuf] = false;
			}
		}
	}

	void VulkanCommandBufferManager::executeCommandBuffer(Ref<CommandBuffer> cmd)
	{
		this->executeCommandBuffers(1, &cmd);
	}

	VulkanStagingBufferHandle VulkanCommandBufferManager::getStagingBuffer()
	{
		auto threadId = std::this_thread::get_id();
		for (auto& poolInfo : m_pools) {
			if (poolInfo.threadId == threadId) {
				return poolInfo.stagingBuffer;
			}
		}
		PE_CORE_ASSERT(false, "No command buffer use that thread!");
		return nullptr;
	}

}
