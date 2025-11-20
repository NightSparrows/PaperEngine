#pragma once

#include <PaperEngine/graphics/Texture.h>

namespace PaperEngine {

	/// <summary>
	/// 只用來載入材質，沒其他的
	/// </summary>
	class TextureLoader {
	public:
		struct TextureConfig {
			bool forceSRGB = false;
			bool generateMipMaps = true;

			TextureConfig() : forceSRGB(false), generateMipMaps(true) {}
		};

		PE_API TextureLoader();

		/// <summary>
		/// 載入原始圖片（PNG, JPG之類的）
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <param name="config"></param>
		/// <returns></returns>
		PE_API TextureHandle load2DFromMemory(nvrhi::CommandListHandle cmd, const void* data, size_t size, const TextureLoader::TextureConfig& config = TextureLoader::TextureConfig());

	public:
		static uint32_t GetMipLevels(uint32_t width, uint32_t height);

	};

}
