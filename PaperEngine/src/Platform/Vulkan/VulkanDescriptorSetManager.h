#pragma once

#include <unordered_map>

#include <PaperEngine/core/Base.h>

#include "VulkanDescriptorSetLayout.h"

namespace PaperEngine {

	class VulkanDescriptorSet;

	class VulkanDescriptorSetManager {
	public:
		~VulkanDescriptorSetManager();

		Ref<VulkanDescriptorSet> allocate(VulkanDescriptorSetLayoutHandle setLayout);

	protected:
		friend class VulkanDescriptorSet;
		void free(VulkanDescriptorSetLayoutHandle layout, VkDescriptorPool pool, VkDescriptorSet set);

		Ref<VulkanDescriptorSet> allocateFromNewPool(VulkanDescriptorSetLayoutHandle setLayout);

	protected:


		struct PoolInfo {
			uint32_t free = 0;
			std::unordered_map<VkDescriptorSet, bool> sets;
		};
		std::unordered_map<VulkanDescriptorSetLayoutHandle, std::unordered_map<VkDescriptorPool, PoolInfo>> m_pools;

		uint32_t m_maxSets = 50;

	};

	typedef Ref<VulkanDescriptorSetManager> VulkanDescriptorSetManagerHandle;
}
