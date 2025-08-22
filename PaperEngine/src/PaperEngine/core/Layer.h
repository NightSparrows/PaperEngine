#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/events/Event.h>
#include <PaperEngine/core/Timestep.h>

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	class Layer {
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void onAttach() {}
		virtual void onDetach() {}

		/// <summary>
		/// Logic update, not rendering.
		/// </summary>
		/// <param name="deltaTime"></param>
		virtual void onUpdate(Timestep deltaTime) {}

		/// <summary>
		/// 用於可以直接在該frame渲染的東西
		/// swapchainIndex已經更新了
		/// 但不會寫入swapchainImage（因為swapchainImage不保證準備好）
		/// </summary>
		virtual void onPreRender() {}

		/// <summary>
		/// 可以寫入swapchainImage的渲染。
		/// 其framebuffer只有swapchain image和一個depth buffer
		/// </summary>
		virtual void onFinalRender(nvrhi::IFramebuffer* framebuffer) {}

		/// <summary>
		/// I/O等事件處理。
		/// </summary>
		/// <param name=""></param>
		virtual void onEvent(Event&) {}

		/// <summary>
		/// ImGui function（optional）
		/// </summary>
		virtual void onImGuiRender() {}

		/// <summary>
		/// Swapchain準備要Resize時
		/// </summary>
		virtual void onBackBufferResizing() {}

		/// <summary>
		/// Swapchain resize完成後
		/// </summary>
		virtual void onBackBufferResized() {}

	};

}
