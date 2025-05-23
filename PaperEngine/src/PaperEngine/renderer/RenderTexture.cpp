
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

	PE_API TextureHandle RenderTexture::get_texture(uint32_t index)
	{
		if (m_impl->isSwapchain) {
			return Application::Get().get_window().get_context().get_swapchain_texture(index);
		}
		return m_impl->textures[index];
	}

	PE_API void RenderTexture::change_size(uint32_t width, uint32_t height)
	{
		m_textureSpec.width = width;
		m_textureSpec.height = height;
		uint32_t imageCount = Application::Get().get_window().get_context().get_swapchain_image_count();
		m_impl->textures.clear();
		for (uint32_t i = 0; i < imageCount; i++) {
			m_impl->textures.push_back(Texture::Create(m_textureSpec));
		}
	}

	Ref<RenderTexture> RenderTexture::Create(const RenderTextureSpecification& spec)
	{
		RenderTexture* res = new RenderTexture(spec);
		return Ref<RenderTexture>(res);
	}

	Ref<RenderTexture> RenderTexture::CreateSwapchainRenderTexture()
	{
		RenderTexture* res = new RenderTexture();
		return Ref<RenderTexture>(res);
	}

	RenderTexture::RenderTexture(const RenderTextureSpecification& spec) :
		m_impl(new RenderTextureImpl())
	{
		m_textureSpec.width = spec.width;
		m_textureSpec.height = spec.height;
		m_textureSpec.format = TextureFormat::RGBA8;
		m_textureSpec.type = TextureType::Texture2D;
		m_textureSpec.isRenderTarget = true;
		uint32_t imageCount = Application::Get().get_window().get_context().get_swapchain_image_count();
		for (uint32_t i = 0; i < imageCount; i++) {
			m_impl->textures.push_back(Texture::Create(m_textureSpec));
		}
	}

	RenderTexture::RenderTexture() :
		m_impl(new RenderTextureImpl())
	{
		m_impl->isSwapchain = true;
	}

}
