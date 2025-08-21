

#include <PaperEngine/PaperEngine.h>

class SandboxApp : public PaperEngine::Application {
public:
	SandboxApp(const PaperEngine::ApplicationProps& spec = PaperEngine::ApplicationProps()) : Application(spec) {

	}
	~SandboxApp() {
	}

	void onInit() override {

	}

private:
};

PaperEngine::Application* PaperEngine::CreateApplication(int argc, const char** argv) {
	PaperEngine::ApplicationProps spec;
	spec.name = "Sandbox";
	return new SandboxApp(spec);
}