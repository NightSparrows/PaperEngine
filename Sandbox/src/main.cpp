

#include <PaperEngine/PaperEngine.h>

class SandboxApp : public PaperEngine::Application {
public:
	SandboxApp(const PaperEngine::ApplicationProps& spec = PaperEngine::ApplicationProps()) : Application(spec) {

	}
	~SandboxApp() {
	}

private:
};

PaperEngine::Application* PaperEngine::CreateApplication(int argc, char** argv) {
	PaperEngine::ApplicationProps spec;
	spec.name = "Sandbox";
	return new SandboxApp(spec);
}