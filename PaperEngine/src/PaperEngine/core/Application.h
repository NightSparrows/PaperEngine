#pragma once

#include <PaperEngine/core/Window.h>

namespace PaperEngine {

	struct ApplicationSpecification
	{
		std::string name = "Application";
		std::string workingDirectory;
		uint32_t width = 1600;
		uint32_t height = 900;
	};

	class PE_API Application {
	public:
		Application(const ApplicationSpecification& spec);
		virtual ~Application() = default;


		void run();

		void on_event(Event& e);


	private:
		bool m_running{ false };

		Scope<Window> m_window;
	};

	// To be defined in CLIENT
	Application* CreateApplication(int argc, char** argv);

}
