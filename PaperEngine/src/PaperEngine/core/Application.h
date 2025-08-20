#pragma once

#include <string>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Window.h>

namespace PaperEngine {

	struct ApplicationProps {
		std::string name = "Application";
		std::string workingDirectory;
		uint32_t width = 2560;
		uint32_t height = 1440;
		// bool disableMinimizeBox = false;
	};

	class Application {
	public:
		PE_API Application(const ApplicationProps& props = ApplicationProps());
		PE_API virtual ~Application();

		PE_API void run();

	protected:
		void onEvent(Event& e);

	private:
		bool m_running = false;

		Scope<Window> m_window;

	private:
		PE_API static Application* s_instance;

	};

	// To be defined in CLIENT
	Application* CreateApplication(int argc, char** argv);

}

