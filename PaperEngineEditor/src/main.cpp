
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
#include <PaperEngine/core/Keyboard.h>
#include <PaperEngine/component/TransformComponent.h>
#include <PaperEngine/imgui/ImGuiUtils.h>
#include <PaperEngine/component/TagComponent.h>

#include "editor/SceneHierarchyPanel.h"

class TestLayer : public PaperEngine::Layer {
public:
	virtual ~TestLayer(){}

	void on_attach() override {
		sceneRenderer = PaperEngine::SceneRenderer::Create({.width = 2560, .height = 1440});
		sceneRenderer->addRenderer(PaperEngine::MeshRenderer::Create());

		// just for testing
		m_activeScene = PaperEngine::CreateRef<PaperEngine::Scene>();

		m_sceneHierarchyPanel.set_context(m_activeScene);

#pragma region Camera initialization demo for main camera

		camEntity = m_activeScene->create_entity();
		auto& tagCom = camEntity.get_component<PaperEngine::TagComponent>();
		tagCom.name = "Main Camera";
		auto& cameraCom = camEntity.add_component<PaperEngine::CameraComponent>();
		//cameraCom.target = PaperEngine::RenderTexture::CreateSwapchainRenderTexture();
		PaperEngine::RenderTextureSpecification renderTextureSpec;
		renderTextureSpec.width = 2560;
		renderTextureSpec.height = 1440;
		m_sceneRenderTexture = PaperEngine::RenderTexture::Create(renderTextureSpec);
		cameraCom.target = m_sceneRenderTexture;

		// for editor scene showing
		m_textureIDs.resize(PaperEngine::Application::Get().get_window().get_context().get_swapchain_image_count());
		for (uint32_t i = 0; i < m_textureIDs.size(); i++) {
			auto texture = cameraCom.target->get_texture(i);
			m_textureIDs[i] = PaperEngine::ImGuiUtils::GetImGuiTexture(texture);
		}

		cameraCom.cameraOrder = 1000;
		
#pragma endregion

		auto testVertexShader = PaperEngine::Shader::Create();
		testVertexShader->load_from_file("../../Sandbox/assets/shaders/test/vert.spv");

		auto testFragmentShader = PaperEngine::Shader::Create();
		testFragmentShader->load_from_file("../../Sandbox/assets/shaders/test/frag.spv");

		PaperEngine::GraphicsPipelineSpecification graphicsPipelineSpec;
		graphicsPipelineSpec.setVertexShader(testVertexShader);
		graphicsPipelineSpec.setFragmentShader(testFragmentShader);
		graphicsPipelineSpec.setCullMode(PaperEngine::GraphicsPipelineSpecification::CullMode::None);
		graphicsPipelineSpec.setCacheFilePath("../../Sandbox/assets/shaders/test/cache.bin");
		//graphicsPipelineSpec.addMaterialUniformBuffer(
		//	0,
		//	PaperEngine::ShaderStage::Fragment,
		//	20
		//);
		graphicsPipelineSpec.addMaterialTexture(
			0,
			PaperEngine::ShaderStage::Fragment
		);

		auto graphicsPipeline = PaperEngine::GraphicsPipeline::Create(graphicsPipelineSpec);

		PaperEngine::MeshData meshData;
		meshData.type = PaperEngine::MeshType::Static;
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-50.f, -50.f, 0}, {0, 0, 0 }, {0, 0} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {-50.f, 50.f, 0}, {0, 0, 0 }, {0, 1.0f} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {50.f, 50.f, 0}, {0, 0, 0 }, {1.0f, 1.0f} });
		meshData.basicVertexData.emplace_back(PaperEngine::MeshData::BasicVertexData{ {50.f, -50.f, 0}, {0, 0, 0 }, {1.0f, 0.0f} });
		meshData.indexData.emplace_back(0);
		meshData.indexData.emplace_back(1);
		meshData.indexData.emplace_back(2);
		meshData.indexData.emplace_back(0);
		meshData.indexData.emplace_back(3);
		meshData.indexData.emplace_back(2);
		meshData.subMeshData.emplace_back(0, 6, 0);

		PaperEngine::MeshHandle mesh = PaperEngine::Mesh::Create();
		mesh->load_mesh_data(meshData);
		
		meshEntity = m_activeScene->create_entity();
		auto& meshEntityTag = meshEntity.get_component<PaperEngine::TagComponent>();
		meshEntityTag.name = "Mesh Entity";
		auto& meshCom = meshEntity.add_component<PaperEngine::MeshComponent>();
		meshCom.mesh = mesh;
		meshCom.materials.push_back(PaperEngine::Material::Create(PaperEngine::MaterialSpec{ .graphicsPipeline = graphicsPipeline }));

		PaperEngine::TextureHandle texture = PaperEngine::Texture::Load2DFromFile("../../Sandbox/assets/model/stallTexture.png");

		meshCom.materials[0]->updateTexture(0, texture);

		// test loading mmd
		//PaperEngine::MMDModelHandle mmdModel = PaperEngine::MMDModel::Create();
		//mmdModel->load("../../Sandbox/assets/model/kisaki/kisaki.pmx");
	}

	void on_detach() override {
		m_activeScene.reset();
		sceneRenderer.reset();
		for (uint32_t i = 0; i < m_textureIDs.size(); i++) {
			PaperEngine::ImGuiUtils::FreeImGuiTexture(m_textureIDs[i]);
		}
		m_textureIDs.clear();
		m_sceneRenderTexture.reset();
		m_sceneHierarchyPanel.set_context(nullptr);
	}

	void on_update(PaperEngine::Timestep delta_time) override {

		// camera movement
		if (m_sceneViewportFocused)
		{
			glm::vec3 moveVector(0);
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::W)) {
				moveVector.z -= 1.f;

			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::S)) {
				moveVector.z += 1.f;
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::A)) {
				moveVector.x -= 1.f;
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::D)) {
				moveVector.x += 1.f;
			}

			if (glm::length(moveVector) != 0) {
				moveVector = glm::normalize(moveVector) * delta_time.to_seconds() * 100.f;

				auto& transform = camEntity.get_component<PaperEngine::TransformComponent>().transform;
				transform.set_position(transform.get_position() + moveVector);
			}
		}

		sceneRenderer->renderScene(*m_activeScene);

		// 2d renderer for gui?

	}

	void on_event(PaperEngine::Event& e) {
		PaperEngine::EventDispatcher dispatcher(e);
		dispatcher.dispatch<PaperEngine::KeyPressedEvent>([](PaperEngine::KeyPressedEvent& e) {
			if (e.get_key_code() == PaperEngine::Key::Escape) {
				PaperEngine::Application::Shutdown();
			}
			return false;
			});
	}

	void on_imgui_render() override {

		// Dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen) {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
			windowFlags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockspaceOpen, windowFlags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{

				if (ImGui::MenuItem("Exit", "")) {
					PaperEngine::Application::Shutdown();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// inside dockspace

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_sceneViewportFocused = ImGui::IsWindowFocused();
		if (m_sceneViewportSize != viewportSize && (viewportSize.x != 0 && viewportSize.y != 0)) {
			m_sceneViewportSize = viewportSize;
			m_sceneRenderTexture->resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			sceneRenderer->resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			auto cameraCom = camEntity.try_get_component<PaperEngine::CameraComponent>();
			cameraCom->camera.set_viewport(viewportSize.x, viewportSize.y);
			if (cameraCom) {
				for (uint32_t i = 0; i < m_textureIDs.size(); i++) {
					PaperEngine::ImGuiUtils::FreeImGuiTexture(m_textureIDs[i]);
					auto texture = cameraCom->target->get_texture(i);
					m_textureIDs[i] = PaperEngine::ImGuiUtils::GetImGuiTexture(texture);
				}
			}
		}
		else {
			// skip this frame
			auto cameraCom = camEntity.try_get_component<PaperEngine::CameraComponent>();
			if (cameraCom) {
				auto cmd = PaperEngine::CommandBuffer::Create({ .isPrimary = true });
				cmd->open();
				cmd->setTextureState(cameraCom->target->get_texture(PaperEngine::Application::Get().get_window().get_context().get_current_swapchain_index()), PaperEngine::TextureState::ShaderReadOnly);
				cmd->close();
				PaperEngine::Application::Get().get_window().get_context().executeCommandBuffer(cmd);
			}
			ImGui::Image(
				m_textureIDs[PaperEngine::Application::Get().get_window().get_context().get_current_swapchain_index()],
				m_sceneViewportSize);
		}
		ImGui::End();
		ImGui::PopStyleVar();

		// scene hierarchy panel
		m_sceneHierarchyPanel.on_imgui_render();

		ImGui::End();
	}
private:
	
	PaperEngine::Ref<PaperEngine::Scene> m_activeScene;
	PaperEngine::Ref<PaperEngine::SceneRenderer> sceneRenderer;
	PaperEngine::Entity camEntity;
	PaperEngine::Entity meshEntity;

	ImVec2 m_sceneViewportSize;
	PaperEngine::RenderTextureHandle m_sceneRenderTexture;
	bool m_sceneViewportFocused{ false };

	std::vector<ImTextureID> m_textureIDs;

	PaperEngine::SceneHierarchyPanel m_sceneHierarchyPanel;
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
	PaperEngine::ApplicationSpecification spec;
	spec.name = "PaperEngine Editor";
	return new SandboxApp(spec);
}