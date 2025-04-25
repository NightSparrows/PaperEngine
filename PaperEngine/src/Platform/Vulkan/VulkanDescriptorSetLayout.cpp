#include "VulkanDescriptorSetLayout.h"

#include <unordered_set>

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "VulkanShader.h"

namespace PaperEngine {
	
	VkDescriptorType VulkanDescriptorSetLayout::ConvertType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::UniformBuffer:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::StorageBuffer:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::CombinedImageSampler:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default:
			PE_CORE_ASSERT(false, "Unknown descriptor type failed to convert.");
			return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const DescriptorSetLayoutSpec& spec)
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings;

		std::unordered_set<uint32_t> boundBindings;

		for (const auto& bindingInfo : spec.bindings) {
			auto& binding = bindings.emplace_back();
			binding.binding = bindingInfo.binding;

			PE_CORE_ASSERT(boundBindings.find(binding.binding) == boundBindings.end(), "Binding slot is already bound.");
			boundBindings.insert(binding.binding);
			
			binding.descriptorCount = bindingInfo.count;
			binding.stageFlags = VulkanShader::ConvertStages(bindingInfo.stages);
			binding.descriptorType = ConvertType(bindingInfo.type);

			auto& poolSize = m_poolSizes.emplace_back();
			poolSize.descriptorCount = binding.descriptorCount;
			poolSize.type = binding.descriptorType;
		}

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(VulkanContext::GetDevice(), &createInfo, nullptr, &m_handle));
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(VulkanContext::GetDevice(), m_handle, nullptr);
	}
}
