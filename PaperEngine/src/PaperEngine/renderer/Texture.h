#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

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
		uint32_t width;
		uint32_t height;
		TextureType type = Texture2D;
		TextureFormat format = TextureFormat::RGBA8;
		bool isRenderTarget{ false };
		bool canBeTransferSrc{ false };
	};

	typedef enum class TextureState {
		TransferDst,
		TransferSrc,
		Present				// only use in swapchain buffer
	}TextureState;

	class Texture {
	public:
		virtual ~Texture() = default;

		virtual uint32_t get_width() const = 0;
		virtual uint32_t get_height() const = 0;

		PE_API static Ref<Texture> Create(const TextureSpecification& spec);
	protected:
		Texture() = default;


	};

	typedef Ref<Texture> TextureHandle;
}
