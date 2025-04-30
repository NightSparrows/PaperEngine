#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/renderer/Texture.h>

#include <PaperEngine/renderer/CommandBuffer.h>

namespace PaperEngine {
	class GraphicsContext
	{
	public:
		GraphicsContext() = default;
		virtual ~GraphicsContext() = default;
		GraphicsContext(const GraphicsContext&) = delete;
		GraphicsContext& operator=(const GraphicsContext&) = delete;

		virtual void init() = 0;

		virtual void cleanUp() = 0;

		virtual bool beginFrame() = 0;

		/// <summary>
		/// How many buffer swapchain have
		/// </summary>
		/// <returns></returns>
		virtual uint32_t get_swapchain_image_count() = 0;

		/// <summary>
		/// Get current swapchain buffer index that will be present
		/// this current frame
		/// </summary>
		/// <returns></returns>
		virtual uint32_t get_current_swapchain_index() = 0;

		virtual TextureHandle get_swapchain_texture(uint32_t swapchainIndex) = 0;

		virtual void executeCommandBuffer(CommandBufferHandle cmd) = 0;

		virtual void executeCommandBuffers(uint32_t count, CommandBufferHandle* cmd) = 0;

		/// <summary>
		/// Get the texture object that swapchain has
		/// </summary>
		/// <param name="image_index"></param>
		/// <returns></returns>
		//virtual TextureHandle get_swapchain_texture(uint32_t image_index) = 0;

		virtual void endFrame() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}