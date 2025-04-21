#pragma once

#include <PaperEngine/core/Window.h>
#include <PaperEngine/events/ApplicationEvent.h>

#include <PaperEngine/core/LayerManager.h>

//#include <PaperEngine/imgui/ImGuiLayer.h>

namespace PaperEngine {

	struct ApplicationSpecification
	{
		std::string name = "Application";
		std::string workingDirectory;
		uint32_t width = 1600;
		uint32_t height = 900;
	};

	class Application {
	public:
		PE_API Application(const ApplicationSpecification& spec);
		PE_API virtual ~Application();


		PE_API void run();

		/// <summary>
		/// Application does not hold the layer for you, 
		/// you need to manager the layer yourself
		/// do not delete while the application is using it
		/// </summary>
		/// <param name="layer"></param>
		/// <returns></returns>
		PE_API void push_layer(Layer* layer);

		PE_API void push_overlay(Layer* layer);

		/// <summary>
		/// Get the window class
		/// </summary>
		/// <returns></returns>
		inline PE_API Window& get_window() { return *m_window; }

		/// <summary>
		/// Get the instance application of this project
		/// </summary>
		/// <returns></returns>
		inline static Application& Get() { return *s_instance; }

	protected:
		void on_event(Event& e);

		bool on_window_resize(WindowResizeEvent& e);

	private:
		bool m_running{ false };

		Scope<Window> m_window;

		LayerManager m_layerManager;

		//ImGuiLayer* m_imguiLayer{ nullptr };

	private:
		static Application* s_instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication(int argc, char** argv);

}
