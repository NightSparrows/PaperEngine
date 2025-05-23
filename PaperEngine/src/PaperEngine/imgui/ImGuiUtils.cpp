#include "ImGuiUtils.h"

#include <PaperEngine/core/Application.h>
#include <backends/imgui_impl_vulkan.h>

// vulkan
#include <Platform/Vulkan/VulkanTexture.h>

#include "ImGuiLayer.h"

namespace PaperEngine {

    PE_API ImTextureID PaperEngine::ImGuiUtils::GetImGuiTexture(TextureHandle texture)
    {
        auto layerInstance = ImGuiLayer::GetInstance();
        if (!layerInstance)
            return 0;

		return layerInstance->addTextureImpl(texture);
    }

    PE_API void ImGuiUtils::FreeImGuiTexture(ImTextureID texture)
    {
        auto layerInstance = ImGuiLayer::GetInstance();
        if (!layerInstance)
            return;

        return layerInstance->freeTextureImpl(texture);
    }

}
