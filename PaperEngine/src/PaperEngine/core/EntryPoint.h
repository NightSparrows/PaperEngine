#pragma once

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/core/Application.h>

extern PaperEngine::Application* PaperEngine::CreateApplication(int argc, char** argv);

namespace PaperEngine {

	int PaperMain(int argc, char** argv) {

		// PaperEngine::Init();
		Logger::Init();

		Application* app = CreateApplication(argc, argv); // This function Write in application
		app->run();
		delete app;

		// PaperEngine::CleanUp();

		return 0;
	}
}

#ifdef PE_DIST

#ifdef PE_PLATFORM_WINDOWS
PE_API int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow) {
	return PaperEngine::PaperMain(__argc, __argv);
}
#endif // PE_PLATFORM_WINDOWS

#else
int main(int argc, char** argv) {
	return PaperEngine::PaperMain(argc, argv);
}

#endif // PE_DIST

