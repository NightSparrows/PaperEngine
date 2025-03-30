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

		}

	}

	void Application::on_event(Event& e)
	{
#ifdef PE_DEBUG
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>([this](Event& e) {
			m_running = false;
			return true;
			});
#endif // PE_DEBUG

	}

	// Explicitly export unique_ptr specialization
	template class PE_API std::unique_ptr<Window>;
}