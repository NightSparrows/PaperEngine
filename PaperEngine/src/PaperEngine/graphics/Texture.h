#pragma once

#include <vector>
#include <cstdint>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	/// <summary>
	/// 使用TextureLoader載入Texture
	/// </summary>
	class Texture {
	public:

		nvrhi::ITexture* getTexture();

	private:
		nvrhi::TextureHandle m_texture;
	};

}
