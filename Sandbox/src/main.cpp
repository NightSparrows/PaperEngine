
#include <iostream>

#include <PaperEngine/PaperEngine.h>

class TestLayer : public PaperEngine::Layer {
public:
	virtual ~TestLayer(){}

	void on_update() override {
	}

	void on_event(PaperEngine::Event& e) {
	}

};

class SandboxApp : public PaperEngine::Application {
public:
	SandboxApp(const PaperEngine::ApplicationSpecification& spec = PaperEngine::ApplicationSpecification()) : Application(spec) {
		m_testLayer = std::make_unique<TestLayer>();
		push_layer(m_testLayer.get());
	}
	~SandboxApp() {
	}

private:
	std::unique_ptr<TestLayer> m_testLayer;
};

PaperEngine::Application* PaperEngine::CreateApplication(int argc, char** argv) {
	return new SandboxApp();
}