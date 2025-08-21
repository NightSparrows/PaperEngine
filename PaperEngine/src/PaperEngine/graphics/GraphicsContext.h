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
		/// 這是present前最後一個CommandList
		/// 
		/// 假設從swapchain Image available
		/// 到present前只有一個commandList需要被執行
		/// 
		/// 這代表會寫入到swapchain image的command只能由這個command執行
		/// 必須在Render裡submit，否則會出問題
		/// 
		/// </summary>
		/// <param name="cmd"></param>
		virtual void submitFinalDrawCmd(nvrhi::CommandListHandle cmd) = 0;

		/// <summary>
		/// 取得Swapchain的數量。
		/// </summary>
		/// <returns></returns>
		virtual uint32_t getSwapchainCount() = 0;

		virtual uint32_t getSwapchainIndex() const = 0;

		/// <summary>
		/// 由於NVRHI的設計缺陷，使用CPU - GPU fence來確保當前
		/// swapchain image可以被使用
		/// </summary>
		virtual void waitForSwapchainImageAvailable() = 0;

		virtual nvrhi::TextureHandle getCurrentSwapchainTexture() = 0;

		virtual nvrhi::DeviceHandle getNVRhiDevice() const = 0;

		virtual nvrhi::FramebufferHandle getCurrentFramebuffer() = 0;



	public:
		static Ref<GraphicsContext> Create(Window* window);
	};

}
