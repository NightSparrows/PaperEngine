
#include <vector>

#include <PaperEngine/core/Application.h>

#include "RenderTexture.h"

namespace PaperEngine {

	struct RenderTextureImpl {
		std::vector<TextureHandle> textures;
		bool isSwapchain{ false };
	};

	RenderTexture::~RenderTexture()
	{
		delete m_impl;
	}

	TextureHandle RenderTexture::get_texture()
	{
		if (m_impl->isSwapchain) {
			return Application::Get().get_window().get_context().get_swapchain_texture(Application::Get().get_window().get_context().get_current_swapchain_index());
		}
		return m_impl->textures[Application::Get().get_window().get_context().get_current_swapchain_index()];
	}

	Ref<RenderTexture> RenderTexture::Create(const TextureSpecification& spec)
	{
		RenderTexture* res = new RenderTexture(spec);
		return Ref<RenderTexture>(res);
	}

	Ref<RenderTexture> RenderTexture::CreateSwapchain()
	{
		RenderTexture* res = new RenderTexture();
		return Ref<RenderTexture>(res);
	}

	RenderTexture::RenderTexture(const TextureSpecification& spec) :
		m_impl(new RenderTextureImpl())
	{
		uint32_t imageCount = Application::Get().get_window().get_context().get_swapchain_image_count();
		for (uint32_t i = 0; i < imageCount; i++) {
			m_impl->textures.push_back(Texture::Create(spec));
		}
	}

	RenderTexture::RenderTexture() :
		m_impl(new RenderTextureImpl())
	{
		m_impl->isSwapchain = true;
	}

}
