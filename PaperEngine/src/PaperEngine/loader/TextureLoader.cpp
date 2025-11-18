#include "TextureLoader.h"

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Application.h>
#include "STBInclude.h"

namespace PaperEngine {
    
    TextureLoader::TextureLoader()
    {
    }

    TextureHandle TextureLoader::load2DFromMemory(nvrhi::CommandListHandle cmd, const void* data, size_t size, const TextureConfig& config)
    {
        int width = 0, height = 0, originalChannels = 0, channels = 0;

        if (stbi_info_from_memory(
            static_cast<const stbi_uc*>(data), 
            static_cast<int>(size), 
            &width, 
            &height, 
            &originalChannels) == 0) {
            PE_CORE_ERROR("Failed to process image header: {}", stbi_failure_reason());
            return nullptr;
        }

        bool isHdr = stbi_is_hdr_from_memory(
            static_cast<const stbi_uc*>(data),
            static_cast<int>(size));

        if (originalChannels == 3) {
            channels = 4;
        }
        else {
            channels = originalChannels;
        }

        uint8_t* bitmap;
        int bytesPerPixels = channels * (isHdr ? 4 : 1);

        if (isHdr) {
            float* floatmap = stbi_loadf_from_memory(
                static_cast<const stbi_uc*>(data),
                static_cast<int>(size),
                &width, &height, &originalChannels, channels);

            bitmap = reinterpret_cast<uint8_t*>(floatmap);
        }
        else {
            bitmap = stbi_load_from_memory(
                static_cast<const stbi_uc*>(data),
                static_cast<int>(size),
                &width, &height, &originalChannels, channels);
        }

        if (!bitmap) {
            PE_CORE_ERROR("Failed to load texture using stb_image");
            return nullptr;
        }

        nvrhi::Format imageFormat;
        switch (channels) {
        case 1:
            imageFormat = isHdr ? nvrhi::Format::R32_FLOAT : nvrhi::Format::R8_UNORM;
            break;
        case 2:
            imageFormat = isHdr ? nvrhi::Format::RG32_FLOAT : nvrhi::Format::RG8_UNORM;
            break;
        case 4:
            imageFormat = isHdr ? nvrhi::Format::RGBA32_FLOAT :
                (config.forceSRGB ? nvrhi::Format::SRGBA8_UNORM : nvrhi::Format::RGBA8_UNORM);
            break;
        default:
            stbi_image_free(bitmap);
            PE_CORE_ERROR("Unknown channels for image: {}", channels);
            return nullptr;
        }

        nvrhi::TextureDesc desc;
        desc.setDebugName("TextureLoader_load_shader_resource");
        desc.format = imageFormat;
        desc.width = width;
        desc.height = height;
        desc.depth = 1;
        desc.arraySize = 1;
        desc.dimension = nvrhi::TextureDimension::Texture2D;
        desc.mipLevels = config.generateMipMaps ? GetMipLevels(width, height) : 1;
        desc.initialState = nvrhi::ResourceStates::ShaderResource;
		desc.keepInitialState = true;       // static image
        desc.isUAV = true;
        TextureHandle texture = CreateRef<Texture>(desc);

        cmd->beginTrackingTextureState(
            texture->getTexture(), 
            nvrhi::AllSubresources, 
            nvrhi::ResourceStates::Common);

        // 寫入原始圖片
        cmd->writeTexture(
            texture->getTexture(),
            0,
            0,
            bitmap,
            static_cast<size_t>(width * bytesPerPixels));

        stbi_image_free(bitmap);

        // TODO generate mipmap using compute shader
        for (uint32_t mipLevel = 1; mipLevel < desc.mipLevels; mipLevel++) {

        }

        cmd->setPermanentTextureState(
            texture->getTexture(),
            nvrhi::ResourceStates::ShaderResource);
        cmd->commitBarriers();


        return texture;
    }

    uint32_t TextureLoader::GetMipLevels(uint32_t width, uint32_t height)
    {
        uint32_t size = std::min(width, height);
        uint32_t levelsNum = (uint32_t)(logf((float)size) / logf(2.0f)) + 1;

        return levelsNum;
    }

}
