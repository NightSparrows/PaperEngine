#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	struct ImageOffset {
		int32_t x;
		int32_t y;
		int32_t z;
	};

	struct ImageExtent {
		uint32_t width;
		uint32_t height;
		uint32_t depth;

		ImageExtent(uint32_t w, uint32_t h, uint32_t d) {
			width = w;
			height = h;
			depth = d;
		}
	};

	typedef enum TextureType {
		Texture2D,
	}TextureType;

	typedef enum class TextureFormat {
		RGBA8,
		RGBA32F,
		Depth,				// the depth format support by the GPU
		Present,			// the format that swapchain support
	}TextureFormat;

	struct TextureSpecification {
		uint32_t width{ 0 };
		uint32_t height{ 0 };
		TextureType type = Texture2D;
		TextureFormat format = TextureFormat::RGBA8;
		bool isRenderTarget{ false };
		bool canBeTransferSrc{ false };
	};

	typedef enum class TextureState {
		TransferDst,
		TransferSrc,
		ShaderReadOnly,
		Present				// only use in swapchain buffer
	}TextureState;

	class Texture {
	public:
		virtual ~Texture() = default;

		PE_API static Ref<Texture> Load2DFromFile(const std::string& filePath, bool genMipmap = true);

		/// <summary>
		/// Load the texture from raw data
		/// </summary>
		/// <param name="texture"></param>
		/// <param name="data"></param>
		/// <param name="offset"></param>
		/// <param name="extent"></param>
		/// <returns></returns>
		PE_API static void Load(Ref<Texture> texture, const void* data, const ImageOffset& offset, const ImageExtent& extent);

		virtual uint32_t get_width() const = 0;
		virtual uint32_t get_height() const = 0;

		PE_API static Ref<Texture> Create(const TextureSpecification& spec);
	protected:
		Texture() = default;


	};

	typedef Ref<Texture> TextureHandle;
}
