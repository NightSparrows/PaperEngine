#include "GraphicsPipeline.h"

#include <Platform/Vulkan/VulkanGraphicsPipeline.h>

namespace PaperEngine {

    Ref<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineSpecification& spec)
    {
        return CreateRef<VulkanGraphicsPipeline>(spec);
    }

}
