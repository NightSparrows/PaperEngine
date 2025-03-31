#include "Application.h"

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/events/ApplicationEvent.h>

namespace PaperEngine {

	

	Application::Application(const ApplicationSpecification& spec)
	{

		m_window = Window::Create(WindowProps(spec.name, spec.width, spec.height));
		m_window->set_event_callback(PE_BIND_EVENT_FN(Application::on_event));

	}

	void Application::run()
	{
		m_running = true;

		while (m_running) {

			m_window->on_update();

			// render
			m_window->get_context()->beginFrame();

			// render logic

			m_window->get_context()->endFrame();

		}

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

	}

	bool Application::on_window_resize(WindowResizeEvent& e)
	{
		return false;
	}

	// Explicitly export unique_ptr specialization
	template class PE_API std::unique_ptr<Window>;
}