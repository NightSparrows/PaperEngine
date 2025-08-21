#pragma once

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/core/Application.h>

extern PaperEngine::Application* PaperEngine::CreateApplication(int argc, const char** argv);

namespace PaperEngine {

	int PaperMain(int argc, const char** argv) {

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
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPreInst, LPSTR lpCmdLine, int nCmdShow) {
#else
int main(int __argc, const char** __argv) {
#endif // PE_PLATFORM_WINDOWS
	return PaperEngine::PaperMain(__argc, __argv);
}

#else
int main(int argc, const char** argv) {
	return PaperEngine::PaperMain(argc, argv);
}

#endif // PE_DIST

