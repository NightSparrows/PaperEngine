#include "Texture.h"

#include <Platform/Vulkan/VulkanTexture.h>

namespace PaperEngine {

    Ref<Texture> PaperEngine::Texture::Create(const TextureSpecification& spec)
    {
		return CreateRef<VulkanTexture>(spec);
    }

}
