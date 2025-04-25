#include "DescriptorSet.h"

#include <Platform/Vulkan/VulkanContext.h>

namespace PaperEngine {

    Ref<DescriptorSet> DescriptorSet::Allocate(DescriptorSetLayoutHandle setLayout)
    {
        return VulkanContext::GetDescriptorSetManager()->allocate(std::static_pointer_cast<VulkanDescriptorSetLayout>(setLayout));
    }

}
