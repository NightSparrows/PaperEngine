
#include <iostream>

#include <PaperEngine/PaperEngine.h>
#include <PaperEngine/scene/Entity.h>

class TestLayer : public PaperEngine::Layer {
public:
	virtual ~TestLayer(){}

	void on_attach() override {

	}

	void on_detach() override {
	}

	void on_update(PaperEngine::Timestep delta_time) override {

	}

	void on_event(PaperEngine::Event& e) {
	}

	void on_imgui_render() override {
		//ImGui::Begin("Test Layer");
		//ImGui::Text("Hello, World!");
		//ImGui::End();
	}
private:
	
	PaperEngine::Ref<PaperEngine::Scene> scene;
	PaperEngine::CameraHandle camera;

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