#include "VulkanDescriptorSetManager.h"

#include "VulkanDescriptorSet.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {
	
	VulkanDescriptorSetManager::~VulkanDescriptorSetManager()
	{
		for (const auto& [setLayout, poolMap] : m_pools) {
			for (const auto& [pool, poolInfo] : poolMap) {
				vkDestroyDescriptorPool(VulkanContext::GetDevice(), pool, nullptr);
				PE_CORE_TRACE("[DescriptorSetManager] descriptor pool destroyed. {}", (void*)pool);
			}
		}
	}

	Ref<VulkanDescriptorSet> VulkanDescriptorSetManager::allocate(VulkanDescriptorSetLayoutHandle setLayout)
	{
		auto layoutIt = m_pools.find(setLayout);
		if (layoutIt == m_pools.end()) {
			// create first descriptor pool
			return this->allocateFromNewPool(setLayout);
		}

		auto& poolMap = layoutIt->second;
		for (auto& [pool, poolInfo] : poolMap) {
			if (poolInfo.free == 0)
				continue;
			for (auto& [set, isUsed] : poolInfo.sets) {
				if (isUsed)
					continue;

				isUsed = true;
				poolInfo.free--;
				PE_CORE_TRACE("[DescriptorSetManager] Descriptor set use. {}, pool: {}, free: {}", (void*)set, (void*)pool, poolInfo.free);
				return CreateRef<VulkanDescriptorSet>(setLayout, pool, set);
			}
		}

		return allocateFromNewPool(setLayout);
	}

	Ref<VulkanDescriptorSet> VulkanDescriptorSetManager::allocateFromNewPool(VulkanDescriptorSetLayoutHandle setLayout)
	{

		// copy the poolSize
		auto poolSizes = setLayout->getPoolSizes();
		for (auto& poolSize : poolSizes) {
			poolSize.descriptorCount *= m_maxSets;
		}

		VkDescriptorPoolCreateInfo poolCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = m_maxSets,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data()
		};
		VkDescriptorPool pool;
		CHECK_VK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice(), &poolCreateInfo, nullptr, &pool));
		PE_CORE_TRACE("[DescriptorSetManager] descriptor pool created. {}", (void*)pool);
		auto& poolInfo = m_pools[setLayout][pool];
		poolInfo.free = m_maxSets;

		VkDescriptorSetLayout layout = setLayout->get_handle();
		VkDescriptorSetAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = pool,
			.descriptorSetCount = 1,
			.pSetLayouts = &layout
		};
		VkDescriptorSet set;
		for (uint32_t i = 1; i < m_maxSets; i++) {
			CHECK_VK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &set));
			poolInfo.sets[set] = false;
		}
		CHECK_VK_RESULT(vkAllocateDescriptorSets(VulkanContext::GetDevice(), &allocInfo, &set));
		poolInfo.sets[set] = true;
		poolInfo.free--;
		PE_CORE_TRACE("[DescriptorSetManager] Descriptor set use. {}, pool: {}, free: {}", (void*)set, (void*)pool, poolInfo.free);
		return CreateRef<VulkanDescriptorSet>(setLayout, pool, set);
	}

	void PaperEngine::VulkanDescriptorSetManager::free(VulkanDescriptorSetLayoutHandle layout, VkDescriptorPool pool, VkDescriptorSet set)
	{
		auto layoutIt = m_pools.find(layout);
		if (layoutIt == m_pools.end())
			return;

		auto& poolMap = layoutIt->second;
		auto poolIt = poolMap.find(pool);
		if (poolIt == poolMap.end())
			return;

		auto& poolInfo = poolIt->second;
		auto setIt = poolInfo.sets.find(set);
		if (setIt == poolInfo.sets.end())
			return;
		setIt->second = false;		// set to not in use
		poolInfo.free++;
		PE_CORE_TRACE("[DescriptorSetManager] Descriptor set free. {}, pool: {}, free: {}", (void*)set, (void*)pool, poolInfo.free);
		if (poolInfo.free < m_maxSets)
			return;

		// remove descriptorPool if the set this pool is not in use
		vkDestroyDescriptorPool(VulkanContext::GetDevice(), pool, nullptr);
		PE_CORE_TRACE("[DescriptorSetManager] descriptor pool destroyed. {}", (void*)pool);
		poolMap.erase(pool);
		if (!poolMap.empty())
			return;

		// remove layout
		m_pools.erase(layout);
	}

}
