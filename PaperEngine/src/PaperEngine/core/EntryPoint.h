#pragma once

#include "Base.h"

#include <PaperEngine/core/Core.h>
#include <PaperEngine/core/Application.h>
#include <PaperEngine/debug/Instrumentor.h>

extern PaperEngine::Application* PaperEngine::CreateApplication(int argc, char** argv);

#ifdef PE_PLATFORM_WINDOWS

namespace PaperEngine {

	int Main(int argc, char** argv) {
		// initialization
		PaperEngine::Core::Init();

		Application* app = CreateApplication(argc, argv);

		app->run();

		delete app;

		// clean up
		PaperEngine::Core::CleanUp();
		return 0;
	}

}
#ifdef PE_DIST
PE_API int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return PaperEngine::Main(__argc, __argv);
}
#else
int main(int argc, char** argv) {
	return PaperEngine::Main(argc, argv);
}
#endif


#endif
