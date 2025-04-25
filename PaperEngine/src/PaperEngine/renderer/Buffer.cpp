#include "Buffer.h"

#include <Platform/Vulkan/VulkanBuffer.h>

namespace PaperEngine {

    Ref<Buffer> Buffer::Create(const BufferSpecification& spec)
    {
		return CreateRef<VulkanBuffer>(spec);
    }

}
