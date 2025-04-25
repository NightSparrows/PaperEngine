
#include "DescriptorSetLayout.h"

#include <Platform/Vulkan/VulkanDescriptorSetLayout.h>

namespace PaperEngine {


	Ref<DescriptorSetLayout> DescriptorSetLayout::Create(const DescriptorSetLayoutSpec& spec)
	{
		return CreateRef<VulkanDescriptorSetLayout>(spec);
	}
}