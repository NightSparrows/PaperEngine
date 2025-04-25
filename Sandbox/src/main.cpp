
#include <iostream>

#include <PaperEngine/PaperEngine.h>
#include <PaperEngine/scene/Entity.h>

#include <PaperEngine/renderer/Mesh.h>
#include <PaperEngine/renderer/MeshData.h>
#include <PaperEngine/renderer/Shader.h>
#include <PaperEngine/renderer/GraphicsPipeline.h>
#include <PaperEngine/renderer/CommandBuffer.h>
#include <PaperEngine/renderer/SceneRenderer.h>
#include <PaperEngine/component/CameraComponent.h>

class TestLayer : public PaperEngine::Layer {
public:
	virtual ~TestLayer(){}

	void on_attach() override {
		sceneRenderer = PaperEngine::SceneRenderer::Create({.width = 2560, .height = 1440});

		scene = PaperEngine::CreateRef<PaperEngine::Scene>();

		auto camEntity = scene->create_entity();
		auto& cameraCom = camEntity.add_component<PaperEngine::CameraComponent>();
		cameraCom.target = PaperEngine::RenderTexture::CreateSwapchain();

		auto testVertexShader = PaperEngine::Shader::Create();
		testVertexShader->load_from_file("../../Sandbox/assets/shaders/test/vert.spv");

		auto testFragmentShader = PaperEngine::Shader::Create();
		testFragmentShader->load_from_file("../../Sandbox/assets/shaders/test/frag.spv");

		PaperEngine::GraphicsPipelineSpecification graphicsPipelineSpec;
		graphicsPipelineSpec.setVertexShader(testVertexShader);
		graphicsPipelineSpec.setFragmentShader(testFragmentShader);
		graphicsPipelineSpec.setCacheFilePath("../../Sandbox/assets/shaders/test/cache.bin");
		graphicsPipelineSpec.addMaterialUniformBuffer(
			0,
			PaperEngine::ShaderStage::Fragment,
			20
		);
		graphicsPipelineSpec.addMaterialTexture(
			1,
			PaperEngine::ShaderStage::Fragment
		);

		auto graphicsPipeline = PaperEngine::GraphicsPipeline::Create(graphicsPipelineSpec);

		PaperEngine::MeshData meshData;
		meshData.type = PaperEngine::MeshData::Static;
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-0.5f, -0.5f, 0}, {0, 0, 0 }, {0, 0} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-0.5f, 0.5f, 0}, {0, 0, 0 }, {0, 1.0f} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {0.5f, 0.5f, 0}, {0, 0, 0 }, {1.0f, 1.0f} });
		meshData.indexData.emplace_back(0);
		meshData.indexData.emplace_back(1);
		meshData.indexData.emplace_back(2);

		PaperEngine::MeshHandle mesh = PaperEngine::Mesh::Create();
		mesh->load_mesh_data(meshData);

	}

	void on_detach() override {
		scene.reset();
		sceneRenderer.reset();
	}

	void on_update(PaperEngine::Timestep delta_time) override {

		sceneRenderer->renderScene(*scene);

		PaperEngine::CommandBufferHandle cmd = PaperEngine::CommandBuffer::Create({ .isPrimary = true });

		PaperEngine::TextureHandle swapchainTexture =
			PaperEngine::Application::Get().get_window().get_context().get_swapchain_texture(PaperEngine::Application::Get().get_window().get_context().get_current_swapchain_index());
		cmd->open();
		cmd->setTextureState(swapchainTexture, PaperEngine::TextureState::Present);
		cmd->close();
		
		PaperEngine::Application::Get().get_window().get_context().executeCommandBuffer(cmd);
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
	PaperEngine::Ref<PaperEngine::SceneRenderer> sceneRenderer;

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