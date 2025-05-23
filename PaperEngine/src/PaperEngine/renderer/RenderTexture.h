#pragma once

#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {
	
	struct RenderTextureImpl;

	struct RenderTextureSpecification {
		uint32_t width, height;
	};;

	// yet just textures manage by it
	class RenderTexture {
	public:
		virtual ~RenderTexture();

		// get the texture that render texture give this frame
		PE_API TextureHandle get_texture();

		PE_API TextureHandle get_texture(uint32_t index);

		PE_API void change_size(uint32_t width, uint32_t height);

		PE_API static Ref<RenderTexture> Create(const RenderTextureSpecification& spec);

		/// <summary>
		/// Create the swapchain Render Texture object
		/// </summary>
		/// <returns></returns>
		PE_API static Ref<RenderTexture> CreateSwapchainRenderTexture();

	protected:
		RenderTexture(const RenderTextureSpecification& spec);
		RenderTexture();

		RenderTextureImpl* m_impl;

		TextureSpecification m_textureSpec;
	};

	typedef Ref<RenderTexture> RenderTextureHandle;
}
