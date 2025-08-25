
#include <fstream>

#include <random>

#include <PaperEngine/PaperEngine.h>
#include <PaperEngine/graphics/SceneRenderer.h>
#include <PaperEngine/components/MeshComponent.h>
#include <PaperEngine/components/MeshRendererComponent.h>
#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/core/Mouse.h>
#include <PaperEngine/core/Keyboard.h>

#include <PaperEngine/loader/TextureLoader.h>

class TestLayer : public PaperEngine::Layer {
public:
	TestLayer() : Layer() {
	}

	void onAttach() override {

		camera.setWidth(PaperEngine::Application::Get()->getWindow()->getWidth());
		camera.setHeight(PaperEngine::Application::Get()->getWindow()->getHeight());

		cmd = PaperEngine::Application::GetNVRHIDevice()->createCommandList();

		m_sceneRenderer = PaperEngine::CreateRef<PaperEngine::SceneRenderer>();

		scene = PaperEngine::CreateRef<PaperEngine::Scene>();

		textureLoader = PaperEngine::CreateRef<PaperEngine::TextureLoader>();

#pragma region Test Graphics pipeline creation

		PaperEngine::Ref<PaperEngine::GraphicsPipeline> graphicsPipeline;
		{
			nvrhi::GraphicsPipelineDesc graphicsPipelineDesc;
			graphicsPipelineDesc.setPrimType(nvrhi::PrimitiveType::TriangleList);
			graphicsPipelineDesc.renderState.depthStencilState.depthTestEnable = true;
			graphicsPipelineDesc.renderState.rasterState.cullMode = nvrhi::RasterCullMode::None;

			graphicsPipelineDesc.bindingLayouts.resize(3);
			graphicsPipelineDesc.bindingLayouts[0] = PaperEngine::Application::GetResourceManager()
				->load<PaperEngine::BindingLayout>("SceneRenderer_globalLayout")->handle;
			graphicsPipelineDesc.bindingLayouts[1] = PaperEngine::Application::GetResourceManager()
				->load<PaperEngine::BindingLayout>("MeshRenderer_instanceBufLayout")->handle;

			// vertex shader
			{
				nvrhi::ShaderDesc shaderDesc;
				shaderDesc.debugName = "Test Vertex Shader";
				shaderDesc.entryName = "main_vs";
				shaderDesc.shaderType = nvrhi::ShaderType::Vertex;
				std::ifstream file("assets/shaders/test/shader.vert.spv", std::ios::binary | std::ios::ate);

				size_t fileSize = file.tellg();

				std::vector<uint8_t> shaderData(fileSize);

				file.seekg(0, std::ios::beg);
				file.read(reinterpret_cast<char*>(shaderData.data()), fileSize);


				graphicsPipelineDesc.VS = PaperEngine::Application::GetNVRHIDevice()->createShader(
					shaderDesc,
					shaderData.data(),
					fileSize);
			}

			// pixel (fragment) shader
			{
				nvrhi::ShaderDesc shaderDesc;
				shaderDesc.debugName = "Test Pixel Shader";
				shaderDesc.entryName = "main_ps";
				shaderDesc.shaderType = nvrhi::ShaderType::Pixel;
				std::ifstream file("assets/shaders/test/shader.frag.spv", std::ios::binary | std::ios::ate);

				size_t fileSize = file.tellg();

				std::vector<uint8_t> shaderData(fileSize);

				file.seekg(0, std::ios::beg);
				file.read(reinterpret_cast<char*>(shaderData.data()), fileSize);

				graphicsPipelineDesc.PS = PaperEngine::Application::GetNVRHIDevice()->createShader(
					shaderDesc,
					shaderData.data(),
					fileSize);
			}

			nvrhi::VertexAttributeDesc attributes[] = {
				nvrhi::VertexAttributeDesc()
				.setName("POSITION")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(PaperEngine::StaticVertex, position))
				.setBufferIndex(0)
				.setElementStride(sizeof(PaperEngine::StaticVertex)),
				nvrhi::VertexAttributeDesc()
				.setName("NORMAL")
				.setFormat(nvrhi::Format::RGB32_FLOAT)
				.setOffset(offsetof(PaperEngine::StaticVertex, normal))
				.setBufferIndex(0)
				.setElementStride(sizeof(PaperEngine::StaticVertex)),
				nvrhi::VertexAttributeDesc()
				.setName("TEXCOORD0")
				.setFormat(nvrhi::Format::RG32_FLOAT)
				.setOffset(offsetof(PaperEngine::StaticVertex, texcoord))
				.setBufferIndex(0)
				.setElementStride(sizeof(PaperEngine::StaticVertex))
			};

			graphicsPipelineDesc.inputLayout = PaperEngine::Application::GetNVRHIDevice()->createInputLayout(
				attributes,
				uint32_t(std::size(attributes)),
				graphicsPipelineDesc.VS
				);

			nvrhi::BindingLayoutDesc bindingLayoutDesc;
			bindingLayoutDesc
				.setVisibility(nvrhi::ShaderType::All)
				.addItem(nvrhi::BindingLayoutItem::Texture_SRV(0))
				.addItem(nvrhi::BindingLayoutItem::Sampler(0));
			nvrhi::BindingLayoutHandle bindingLayout = PaperEngine::Application::GetNVRHIDevice()->createBindingLayout(bindingLayoutDesc);
			graphicsPipelineDesc.bindingLayouts[2] = bindingLayout;

			graphicsPipeline = PaperEngine::CreateRef<PaperEngine::GraphicsPipeline>(graphicsPipelineDesc, bindingLayout, 0);
		}

#pragma endregion


#pragma region Test Entity
		{

			cmd->open();
			auto mesh = PaperEngine::CreateRef<PaperEngine::Mesh>();
			std::vector<PaperEngine::StaticVertex> vertices = {
				{{-50.f, 50.f, 0}, {0, 1, 0}, {0, 0}},
				{{-50.f, -50.f, 0}, {0, 1, 0}, {0, 1.0f}},
				{{50.f, -50.f, 0}, {0, 1, 0}, {1.f, 1.f}},
				{{50.f, 50.f, 0}, {0, 1, 0}, {1.f, 0}}
			};
			std::vector<uint32_t> indices = {
				0, 1, 3, 3, 1, 2
			};
			mesh->loadStaticMesh(cmd, vertices);
			mesh->loadIndexBuffer(cmd, indices.data(), indices.size());
			mesh->getSubMeshes().resize(1);
			mesh->getSubMeshes()[0] = { 0, 6 };

			PaperEngine::TextureHandle texture = nullptr;
#pragma region Load test image
			{
				std::ifstream file("assets/test/test.png", std::ios::ate | std::ios::binary);

				if (file.is_open()) {

					size_t fileSize = file.tellg();
					std::vector<uint8_t> imageFileContent(fileSize);

					file.seekg(0, std::ios::beg);
					file.read(reinterpret_cast<char*>(imageFileContent.data()), fileSize);
					file.close();

					PaperEngine::TextureLoader::TextureConfig config;
					config.generateMipMaps = false;
					texture = textureLoader->load2DFromMemory(
						cmd,
						imageFileContent.data(),
						fileSize,
						config);

					cmd->close();
					PaperEngine::Application::GetNVRHIDevice()->executeCommandList(cmd);
				}
				else {
					PE_CORE_ERROR("Failed to open file");
				}

			}
#pragma endregion

			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
			static std::uniform_real_distribution<float> rotDist(0, 1.0f);
			nvrhi::SamplerDesc samplerDesc;
			nvrhi::SamplerHandle sampler = PaperEngine::Application::GetNVRHIDevice()->createSampler(samplerDesc);
			auto material = PaperEngine::CreateRef<PaperEngine::Material>(graphicsPipeline);
			material->setSampler("sampler0", sampler);

			material->setTexture("texture0", texture);
			for (uint32_t i = 0; i < 10000; i++) {
				auto testEntity = scene->createEntity("test Entity");
				auto& testMeshCom = testEntity.addComponent<PaperEngine::MeshComponent>();
				testMeshCom.mesh = mesh;

				auto& meshRendererCom = testEntity.addComponent<PaperEngine::MeshRendererComponent>();
				meshRendererCom.materials.resize(1);

				// TODO graphics pipeline, variables
				meshRendererCom.materials[0] = material;

				auto& transCom = testEntity.getComponent<PaperEngine::TransformComponent>();

				transCom.transform.setPosition(
					glm::vec3(dist(gen), dist(gen), dist(gen))
				);
				// TODO 血一個Entity move的function，
				// transform component跟mesh component裡的aabb
				// 一起更新
				testMeshCom.worldAABB = mesh->getAABB().transformed(transCom.transform);

				float u1 = rotDist(gen);
				float u2 = rotDist(gen);
				float u3 = rotDist(gen);

				float sqrt1MinusU1 = std::sqrt(1.0f - u1);
				float sqrtU1 = std::sqrt(u1);

				float theta1 = 2.0f * glm::pi<float>() * u2;
				float theta2 = 2.0f * glm::pi<float>() * u3;

				float w = sqrt1MinusU1 * std::sin(theta1);
				float x = sqrt1MinusU1 * std::cos(theta1);
				float y = sqrtU1 * std::sin(theta2);
				float z = sqrtU1 * std::cos(theta2);

				transCom.transform.setRotation(glm::quat(w, x, y, z));
			}
		}
#pragma endregion

		cameraTransform.setPosition({ 0, 0, 100 });

		PE_CORE_INFO("TestLayer attached.");
	}
	void onDetach() override {
		cmd = nullptr;
		textureLoader = nullptr;
		scene = nullptr;
		m_sceneRenderer = nullptr;
		PE_CORE_INFO("TestLayer detached.");
	}
	
	void onUpdate(PaperEngine::Timestep dt) override {
		const float SPEED= 100.f;
		const float LOOK_SENSITIVITY = 10.f;
		if (PaperEngine::Mouse::IsMouseButtonDown(PaperEngine::Mouse::ButtonRight)) {
			PaperEngine::Mouse::GrabMouseCursor(true);

			glm::vec3 moveVector(0);
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::W)) {
				moveVector += cameraTransform.getForward();
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::S)) {
				moveVector -= cameraTransform.getForward();
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::A)) {
				moveVector -= cameraTransform.getRight();
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::D)) {
				moveVector += cameraTransform.getRight();
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::E)) {
				moveVector += cameraTransform.getUp();
			}
			if (PaperEngine::Keyboard::IsKeyDown(PaperEngine::Key::Q)) {
				moveVector -= cameraTransform.getUp();
			}

			if (glm::length(moveVector) != 0) {
				moveVector = glm::normalize(moveVector) * dt.toSeconds() * SPEED;

				cameraTransform.setPosition(cameraTransform.getPosition() + moveVector);
				//PE_CORE_INFO("Camera position: {0} {1} {2}", m_transform.get_position().x, m_transform.get_position().y, m_transform.get_position().z);
			}

			glm::vec2 mouseDelta = PaperEngine::Mouse::GetDeltaPosition();
			if (mouseDelta.x != 0 || mouseDelta.y != 0) {
				float yaw = mouseDelta.x * LOOK_SENSITIVITY * dt.toSeconds();
				float pitch = mouseDelta.y * LOOK_SENSITIVITY * dt.toSeconds();
				cameraTransform.rotate(glm::vec3(0, 1, 0), -yaw);
				cameraTransform.rotate(cameraTransform.getRight(), -pitch); // invert pitch to match typical camera controls
			}

		}
		else {
			PaperEngine::Mouse::GrabMouseCursor(false);
		}

	}

	void onImGuiRender() {
		ImGui::Begin("test");
		ImGui::End();
	}

	void onFinalRender(nvrhi::IFramebuffer* framebuffer) override {

		std::vector<PaperEngine::Ref<PaperEngine::Scene>> scenes;
		scenes.push_back(scene);
		m_sceneRenderer->renderScene(scenes, &camera, &cameraTransform, framebuffer);

	}

	void onBackBufferResized() {
		camera.setWidth(PaperEngine::Application::Get()->getWindow()->getWidth());
		camera.setHeight(PaperEngine::Application::Get()->getWindow()->getHeight());
	}
private:
	nvrhi::CommandListHandle cmd;
	PaperEngine::Ref<PaperEngine::SceneRenderer> m_sceneRenderer;

	PaperEngine::Ref<PaperEngine::Scene> scene;
	PaperEngine::Camera camera;
	PaperEngine::Transform cameraTransform;

	PaperEngine::Ref<PaperEngine::TextureLoader> textureLoader;

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