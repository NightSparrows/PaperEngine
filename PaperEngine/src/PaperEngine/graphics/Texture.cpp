#include "Texture.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {

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
