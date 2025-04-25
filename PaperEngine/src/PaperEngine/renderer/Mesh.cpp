#include "Mesh.h"

#include <Platform/Vulkan/VulkanMesh.h>

namespace PaperEngine {

    Ref<Mesh> Mesh::Create()
    {
        return CreateRef<VulkanMesh>();
    }

}
