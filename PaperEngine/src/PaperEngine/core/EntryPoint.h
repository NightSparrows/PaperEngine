#pragma once

#include "Base.h"

#include "Logger.h"

#ifdef PE_PLATFORM_WINDOWS

namespace PaperEngine {

	int Main(int argc, char** argv) {
		Logger::Init();

		return 0;
	}

}
#ifdef PE_DIST
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return PaperEngine::Main(__argc, __argv);
}
#else
int main(int argc, char** argv) {
	return PaperEngine::Main(argc, argv);
}
#endif


#endif
