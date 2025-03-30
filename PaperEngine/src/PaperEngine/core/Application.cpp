#include "Application.h"

#include <PaperEngine/core/Logger.h>

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
	}

	// Explicitly export unique_ptr specialization
	template class PE_API std::unique_ptr<Window>;
}