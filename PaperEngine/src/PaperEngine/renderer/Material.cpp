#include "Material.h"

#include <Platform/Vulkan/VulkanMaterial.h>

namespace PaperEngine {

	Ref<Material> Material::Create(const MaterialSpec& spec)
	{
		return CreateRef<VulkanMaterial>(spec);
	}

}
