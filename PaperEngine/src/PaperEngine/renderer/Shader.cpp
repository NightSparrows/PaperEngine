#include "Shader.h"

#include <Platform/Vulkan/VulkanShader.h>

namespace PaperEngine {

    Ref<Shader> Shader::Create()
    {
        return CreateRef<VulkanShader>();
    }

}
