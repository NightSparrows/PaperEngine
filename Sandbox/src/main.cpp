

#include <PaperEngine/PaperEngine.h>

class SandboxApp : public PaperEngine::Application {
public:
	SandboxApp(const PaperEngine::ApplicationSpecification& spec = PaperEngine::ApplicationSpecification()) : Application(spec) {
	}
};

PaperEngine::Application* PaperEngine::CreateApplication(int argc, char** argv) {
	return new SandboxApp();
}