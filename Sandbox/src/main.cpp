
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
#include <PaperEngine/renderer/MeshRenderer.h>
#include <PaperEngine/events/KeyEvent.h>
#include <PaperEngine/component/MeshComponent.h>

class TestLayer : public PaperEngine::Layer {
public:
	virtual ~TestLayer(){}

	void on_attach() override {
		sceneRenderer = PaperEngine::SceneRenderer::Create({.width = 2560, .height = 1440});
		sceneRenderer->addRenderer(PaperEngine::MeshRenderer::Create());

		scene = PaperEngine::CreateRef<PaperEngine::Scene>();

		camEntity = scene->create_entity();
		auto& cameraCom = camEntity.add_component<PaperEngine::CameraComponent>();
		cameraCom.target = PaperEngine::RenderTexture::CreateSwapchain();

		auto testVertexShader = PaperEngine::Shader::Create();
		testVertexShader->load_from_file("../../Sandbox/assets/shaders/test/vert.spv");

		auto testFragmentShader = PaperEngine::Shader::Create();
		testFragmentShader->load_from_file("../../Sandbox/assets/shaders/test/frag.spv");

		PaperEngine::GraphicsPipelineSpecification graphicsPipelineSpec;
		graphicsPipelineSpec.setVertexShader(testVertexShader);
		graphicsPipelineSpec.setFragmentShader(testFragmentShader);
		graphicsPipelineSpec.setCullMode(PaperEngine::GraphicsPipelineSpecification::CullMode::None);
		graphicsPipelineSpec.setCacheFilePath("../../Sandbox/assets/shaders/test/cache.bin");
		/*graphicsPipelineSpec.addMaterialUniformBuffer(
			0,
			PaperEngine::ShaderStage::Fragment,
			20
		);
		graphicsPipelineSpec.addMaterialTexture(
			1,
			PaperEngine::ShaderStage::Fragment
		);*/

		auto graphicsPipeline = PaperEngine::GraphicsPipeline::Create(graphicsPipelineSpec);

		PaperEngine::MeshData meshData;
		meshData.type = PaperEngine::MeshType::Static;
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-50.f, -50.f, 0}, {0, 0, 0 }, {0, 0} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-50.f, 50.f, 0}, {0, 0, 0 }, {0, 1.0f} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {50.f, 50.f, 0}, {0, 0, 0 }, {1.0f, 1.0f} });
		meshData.indexData.emplace_back(0);
		meshData.indexData.emplace_back(1);
		meshData.indexData.emplace_back(2);
		meshData.subMeshData.emplace_back(0, 3);

		PaperEngine::MeshHandle mesh = PaperEngine::Mesh::Create();
		mesh->load_mesh_data(meshData);
		
		auto meshEntity = scene->create_entity();
		auto& meshCom = meshEntity.add_component<PaperEngine::MeshComponent>();
		meshCom.mesh = mesh;
		meshCom.materials.push_back(PaperEngine::Material::Create(PaperEngine::MaterialSpec{ .graphicsPipeline = graphicsPipeline }));
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
		PaperEngine::EventDispatcher dispatcher(e);
		dispatcher.dispatch<PaperEngine::KeyPressedEvent>([this](PaperEngine::KeyPressedEvent& e) {
			auto& camCom = camEntity.get_component<PaperEngine::CameraComponent>();
			glm::vec3 moveVector(0);
			if (e.get_key_code() == PaperEngine::Key::W) {
				moveVector.z -= 1.f;
			}
			if (e.get_key_code() == PaperEngine::Key::S) {
				moveVector.z += 1.f;
			}
			if (e.get_key_code() == PaperEngine::Key::A) {
				moveVector.x -= 1.f;
			}
			if (e.get_key_code() == PaperEngine::Key::D) {
				moveVector.x += 1.f;
			}
			camCom.camera.set_position(camCom.camera.get_position() + moveVector);
			std::cout << "Position: " << camCom.camera.get_position().x << ", " << camCom.camera.get_position().y << ", " << camCom.camera.get_position().z << "\n";
			return false;
			});
	}

	void on_imgui_render() override {
		//ImGui::Begin("Test Layer");
		//ImGui::Text("Hello, World!");
		//ImGui::End();
	}
private:
	
	PaperEngine::Ref<PaperEngine::Scene> scene;
	PaperEngine::Ref<PaperEngine::SceneRenderer> sceneRenderer;
	PaperEngine::Entity camEntity;

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