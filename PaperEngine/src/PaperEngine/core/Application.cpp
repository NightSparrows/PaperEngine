﻿
#pragma warning(push)
#pragma warning(disable: 4251)

#include "Application.h"

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/events/ApplicationEvent.h>

namespace PaperEngine {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		PE_CORE_ASSERT(!s_instance, "Application already created.");
		s_instance = this;

		m_window = Window::Create(WindowProps(spec.name, spec.width, spec.height));
	}

	Application::~Application()
	{
		delete m_imguiLayer;
	}

	void Application::run()
	{
		m_window->init();
		m_window->set_event_callback(PE_BIND_EVENT_FN(Application::on_event));

		m_imguiLayer = new ImGuiLayer();
		push_overlay(m_imguiLayer);

		m_running = true;

		while (m_running) {

			// update
			m_window->on_update();
			for (auto& layer : m_layerManager) {
				layer->on_update();
			}

			// render
			m_window->get_context().beginFrame();

			// render logic
			m_imguiLayer->begin_frame();
			for (auto& layer : m_layerManager) {
				layer->on_imgui_render();
			}
			m_imguiLayer->end_frame();

			m_window->get_context().endFrame();

		}

		m_layerManager.cleanUp();

		m_window->cleanUp();
	}

	PE_API void Application::push_layer(Layer* layer)
	{
		m_layerManager.push_layer(layer);
		layer->on_attach();
	}

	PE_API void Application::push_overlay(Layer* layer)
	{
		m_layerManager.push_overlay(layer);
		layer->on_attach();
	}

	void Application::on_event(Event& e)
	{
		EventDispatcher dispatcher(e);
#ifdef PE_DEBUG
		dispatcher.dispatch<WindowCloseEvent>([this](Event& e) {
			m_running = false;
			return true;
			});
#endif // PE_DEBUG
		dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {return this->on_window_resize(e); });

		for (auto it = m_layerManager.rbegin(); it != m_layerManager.rend(); ++it) {
			if (e.Handled)
				break;
			(*it)->on_event(e);
		}
	}

	bool Application::on_window_resize(WindowResizeEvent& e)
	{
		return false;
	}

	// Explicitly export unique_ptr specialization
	template class PE_API std::unique_ptr<Window>;
}


#pragma warning(pop)