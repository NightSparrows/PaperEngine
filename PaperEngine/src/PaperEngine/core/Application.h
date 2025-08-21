#pragma once

#include <string>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Window.h>
#include <PaperEngine/graphics/GraphicsContext.h>
#include <PaperEngine/core/LayerManager.h>


//純測試，TODO加到cmake
#define PE_ENABLE_IMGUI

#ifdef PE_ENABLE_IMGUI
#include <PaperEngine/imgui/ImGuiLayer.h>
#endif // PE_ENABLE_IMGUI

namespace PaperEngine {

	enum class RenderAPI {
		Vulkan
	};

	struct ApplicationProps {
		std::string name = "Application";
		std::string workingDirectory;
		uint32_t width = 2560;
		uint32_t height = 1440;
		// bool disableMinimizeBox = false;
		RenderAPI renderAPI = RenderAPI::Vulkan;
	};

	class Application {
	public:
		PE_API Application(const ApplicationProps& props = ApplicationProps());
		PE_API virtual ~Application();

		/// <summary>
		/// 由EntryPoint執行
		/// </summary>
		/// <returns></returns>
		PE_API void run();

		/// <summary>
		/// Used for after all of the engine stuff initialized
		/// </summary>
		virtual void onInit() {}

		PE_API Ref<GraphicsContext> getGraphicsContext() { return m_graphicsContext; }

		PE_API Window* getWindow() { return m_window.get(); }

		inline PE_API RenderAPI getRenderAPI() const { return m_renderAPI; }

	public:
		/// <summary>
		/// 全域Application實例。
		/// 只能有一個Application實例。
		/// </summary>
		/// <returns></returns>
		PE_API static Application* Get() { return s_instance; }

		PE_API static void Shutdown();

	protected:
		void onEvent(Event& e);

		void onBackBufferResizing();

		void onBackBufferResized();

	private:
		bool m_running = false;

		Scope<Window> m_window;
		Ref<GraphicsContext> m_graphicsContext;

		RenderAPI m_renderAPI = RenderAPI::Vulkan;

		LayerManager m_layerManager;

#ifdef PE_ENABLE_IMGUI
		Ref<ImGuiLayer> m_imguiLayer;
#endif // PE_ENABLE_IMGUI


	private:
		PE_API static Application* s_instance;

	};

	// To be defined in CLIENT
	Application* CreateApplication(int argc, const char** argv);

}

