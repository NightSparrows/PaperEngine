

#include <PaperEngine/PaperEngine.h>

#include <nvrhi/utils.h>

class TestLayer : public PaperEngine::Layer {
public:
	TestLayer() : Layer() {
	}

	void onAttach() override {
		cmd = PaperEngine::Application::GetNVRHIDevice()->createCommandList();
		PE_CORE_INFO("TestLayer attached.");
	}
	void onDetach() override {
		cmd = nullptr;
		PE_CORE_INFO("TestLayer detached.");
	}
	void onUpdate(PaperEngine::Timestep ts) override {
		// Update logic here
	}

	void onFinalRender(nvrhi::IFramebuffer* framebuffer) override {
		cmd->open();
		nvrhi::utils::ClearColorAttachment(cmd, framebuffer, 0, nvrhi::Color(0.f));

		cmd->close();
		PaperEngine::Application::GetNVRHIDevice()->executeCommandList(cmd);
	}
private:
	nvrhi::CommandListHandle cmd;
};

class SandboxApp : public PaperEngine::Application {
public:
	SandboxApp(const PaperEngine::ApplicationProps& spec = PaperEngine::ApplicationProps()) : Application(spec) {

	}
	~SandboxApp() {
	}

	void onInit() override {
		pushLayer(&m_testLayer);
	}

private:
	TestLayer m_testLayer;
};

PaperEngine::Application* PaperEngine::CreateApplication(int argc, const char** argv) {
	PaperEngine::ApplicationProps spec;
	spec.name = "Sandbox";
	return new SandboxApp(spec);
}