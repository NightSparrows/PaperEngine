

#include <nvrhi/nvrhi.h>
#include "Application.h"

#include <PaperEngine/core/Assert.h>
#include <PaperEngine/core/Logger.h>


void test() {
}

namespace PaperEngine {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationProps& props)
	{
		PE_CORE_ASSERT(!s_instance, "Application already created.");
		s_instance = this;

		m_window = Window::Create(WindowProps(props.name, props.width, props.height));
		m_window->init();
		m_window->setEventCallback(PE_BIND_EVENT_FN(Application::onEvent));

	}

	Application::~Application()
	{
	}

	PE_API void Application::run()
	{

		m_running = true;

		while (m_running) {

			// update
			{
				m_window->onUpdate();
				// Update logic, input handling, etc.
			}

			// Render
			{
			}

		}

	}

	void Application::onEvent(Event& e)
	{
	}

}
