#include "CommandBuffer.h"

#include <Platform/Vulkan/VulkanCommandBuffer.h>

namespace PaperEngine {

    Ref<CommandBuffer> PaperEngine::CommandBuffer::Create(const CommandBufferSpec& spec)
    {
        return CreateRef<VulkanCommandBuffer>(spec);
    }

}
