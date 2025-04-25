#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/renderer/DescriptorSetLayout.h>

namespace PaperEngine {

	class VulkanDescriptorSetLayout : public DescriptorSetLayout {
	public:
		VulkanDescriptorSetLayout(const DescriptorSetLayoutSpec& spec);
		~VulkanDescriptorSetLayout();

		const std::vector<VkDescriptorPoolSize>& getPoolSizes() const { return m_poolSizes; }

		VkDescriptorSetLayout get_handle() const { return m_handle; }

		static VkDescriptorType ConvertType(DescriptorType type);

	protected:

		VkDescriptorSetLayout m_handle{ VK_NULL_HANDLE };

		// for pool creation
		std::vector< VkDescriptorPoolSize> m_poolSizes;
	};

	typedef Ref<VulkanDescriptorSetLayout> VulkanDescriptorSetLayoutHandle;
}
