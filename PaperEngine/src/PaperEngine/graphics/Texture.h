#pragma once

#include <vector>
#include <cstdint>

#include <nvrhi/nvrhi.h>

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	/// <summary>
	/// 使用TextureLoader載入Texture
	/// </summary>
	class Texture {
	public:
		Texture(nvrhi::TextureDesc desc);
		~Texture();


		nvrhi::ITexture* getTexture();



	private:
		nvrhi::TextureHandle m_texture;
	};

	typedef Ref<Texture> TextureHandle;
}
