#pragma once

#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {
	
	struct RenderTextureImpl;

	// yet just textures manage by it
	class RenderTexture {
	public:
		virtual ~RenderTexture();

		// get the texture that render texture give this frame
		PE_API TextureHandle get_texture();

		PE_API static Ref<RenderTexture> Create(const TextureSpecification& spec);

		/// <summary>
		/// Create the swapchain Render Texture object
		/// </summary>
		/// <returns></returns>
		PE_API static Ref<RenderTexture> CreateSwapchain();

	protected:
		RenderTexture(const TextureSpecification& spec);
		RenderTexture();

		RenderTextureImpl* m_impl;
	};

	typedef Ref<RenderTexture> RenderTextureHandle;
}
