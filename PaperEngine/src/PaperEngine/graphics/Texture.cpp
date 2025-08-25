#include "Texture.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {
    
    Texture::Texture(nvrhi::TextureDesc desc)
    {
        m_texture = Application::GetNVRHIDevice()->createTexture(desc);
        PE_CORE_TRACE("Texture created. {}", (void*)m_texture);
    }

    Texture::~Texture() {
        PE_CORE_TRACE("Texture destroyed. {}", (void*)m_texture);
    }

    nvrhi::ITexture* Texture::getTexture()
    {
        return m_texture;
    }

    //uint32_t Texture::GetMipLevelsNum(uint32_t width, uint32_t height)
    //{
    //    uint32_t size = std::min(width, height);
    //    uint32_t levelsNum = (uint32_t)(logf((float)size) / logf(2.0f)) + 1;

    //    return levelsNum;

    //}

}
