#include "Texture.h"


#pragma warning(push)
#pragma warning(disable : 26819)
#pragma warning(disable : 6262)
#define STB_IMAGE_IMPLEMENTATION
#include <3rdParty/stb/stb_image.h>
#pragma warning(pop)

#include <PaperEngine/core/Application.h>
#include <PaperEngine/core/Logger.h>
#include <PaperEngine/renderer/CommandBuffer.h>

#include <Platform/Vulkan/VulkanTexture.h>



namespace PaperEngine {
    PE_API Ref<Texture> Texture::Load2DFromFile(const std::string& filePath, TextureLoadSpec loadSpec)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* imageData = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!imageData) {
            PE_CORE_ERROR("Failed to load texture: {}", filePath);
            return nullptr;
        }

        TextureSpecification spec;
        spec.width = static_cast<uint32_t>(texWidth);
        spec.height = static_cast<uint32_t>(texHeight);
        spec.format = TextureFormat::RGBA8;
        if (loadSpec.genMipmap) {
            spec.canBeTransferSrc = true;
            // TODO: mip mapping subresource
        }
        spec.type = Texture2D;
        TextureHandle texture = Texture::Create(spec);

        CommandBufferHandle cmd = CommandBuffer::Create({ .isPrimary = true });
        cmd->open();
        cmd->setTextureState(texture, TextureState::TransferDst);
        cmd->writeTexture(texture, imageData, {0, 0, 0}, {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1});
        stbi_image_free(imageData);
        cmd->setTextureState(texture, TextureState::ShaderReadOnly);
        cmd->close();
        Application::Get().get_window().get_context().executeCommandBuffer(cmd);

        return texture;
    }
    void Texture::Load(Ref<Texture> texture, const void* data, const ImageOffset& offset, const ImageExtent& extent)
    {
        CommandBufferHandle cmd = CommandBuffer::Create({.isPrimary = true});
        cmd->open();
        cmd->setTextureState(texture, TextureState::TransferDst);
        cmd->writeTexture(texture, data, offset, extent);
        cmd->setTextureState(texture, TextureState::ShaderReadOnly);
        cmd->close();
        Application::Get().get_window().get_context().executeCommandBuffer(cmd);
    }

    Ref<Texture> PaperEngine::Texture::Create(const TextureSpecification& spec)
    {
		return CreateRef<VulkanTexture>(spec);
    }

}
