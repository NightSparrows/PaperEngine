#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/core/Window.h>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	class GraphicsContext {
	public:
		virtual ~GraphicsContext() = default;

#pragma region Application calling function
		virtual void init() = 0;

		virtual void cleanUp() = 0;

		virtual bool beginFrame() = 0;

		virtual bool present() = 0;
		/// <summary>
		/// 準備Resize時
		/// </summary>
		/// <param name="callback"></param>
		virtual void setOnBackBufferResizingCallback(const std::function<void()>& callback) = 0;

		/// <summary>
		/// Swapchain Resize完成後
		/// </summary>
		/// <param name="callback"></param>
		virtual void setOnBackBufferResizedCallback(const std::function<void()>& callback) = 0;

#pragma endregion

		/// <summary>
		/// 取得Swapchain的數量。
		/// </summary>
		/// <returns></returns>
		virtual uint32_t getSwapchainCount() = 0;

		virtual uint32_t getSwapchainIndex() const = 0;

		virtual nvrhi::TextureHandle getCurrentSwapchainTexture() = 0;

		virtual nvrhi::IDevice* getNVRhiDevice() const = 0;

		virtual nvrhi::FramebufferHandle getCurrentFramebuffer() = 0;

		virtual nvrhi::Format getSupportedDepthFormat() = 0;

		/// <summary>
		/// Frame in flight
		/// </summary>
		virtual uint32_t getMaxFrameInFlight() const = 0;

		virtual uint32_t getCurrentFrameInFlightIndex() const = 0;

		virtual nvrhi::CommandListHandle getCurrentFrameCommandList() = 0;


	public:
		static Ref<GraphicsContext> Create(Window* window);
	};

}
