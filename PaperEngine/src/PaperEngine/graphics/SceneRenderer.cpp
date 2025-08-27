#include "SceneRenderer.h"

#include <PaperEngine/core/Application.h>

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/MeshComponent.h>
#include <PaperEngine/components/MeshRendererComponent.h>
#include <PaperEngine/components/LightComponent.h>
#include <PaperEngine/debug/Instrumentor.h>

#include <PaperEngine/utils/Frustum.h>

#include <nvrhi/utils.h>

#include <execution>

namespace PaperEngine {

	SceneRenderer::SceneRenderer()
	{
		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		// 全域data
		nvrhi::BufferDesc globalDataBufferDesc;
		globalDataBufferDesc
			.setInitialState(nvrhi::ResourceStates::ConstantBuffer)
			.setKeepInitialState(true)
			.setByteSize(sizeof(GlobalDataI))
			.setIsConstantBuffer(true);
		m_globalDataBuffer = Application::GetNVRHIDevice()->createBuffer(globalDataBufferDesc);

		nvrhi::BindingLayoutDesc globalLayoutDesc;
		globalLayoutDesc
			.setRegisterSpace(0)			// set = 0
			.setRegisterSpaceIsDescriptorSet(true)
			.setVisibility(nvrhi::ShaderType::All)
			.addItem(nvrhi::BindingLayoutItem::ConstantBuffer(0));
		m_globalLayout =
			Application::GetResourceManager()->create<BindingLayout>("SceneRenderer_globalLayout",
				Application::GetNVRHIDevice()->createBindingLayout(globalLayoutDesc));

		nvrhi::BindingSetDesc globalSetDesc;
		globalSetDesc.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_globalDataBuffer));
		m_globalSet = Application::GetNVRHIDevice()->createBindingSet(globalSetDesc, m_globalLayout->handle);
	}

	void SceneRenderer::renderScene(std::span<Ref<Scene>> scenes, const Camera* camera, const Transform* transform, nvrhi::IFramebuffer* fb)
	{
		PE_PROFILE_FUNCTION();
		// 
		GlobalDataI globalData{};
		globalData.projectionMatrix = camera->getProjectionMatrix();
		globalData.viewMatrix = glm::inverse(transform->matrix());
		globalData.projViewMatrix = globalData.projectionMatrix * globalData.viewMatrix;
		globalData.cameraPosition = transform->getPosition();
		m_cmd->open();

		nvrhi::utils::ClearColorAttachment(m_cmd, fb, 0, nvrhi::Color(0.f));
		nvrhi::utils::ClearDepthStencilAttachment(m_cmd, fb, 1.f, 0);

		m_cmd->writeBuffer(m_globalDataBuffer, &globalData, sizeof(globalData));

		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);

		GlobalSceneData sceneData;
		sceneData.camera = camera;
		sceneData.cameraTransform = transform;
		sceneData.fb = fb;
		sceneData.globalSet = m_globalSet;
		sceneData.projViewMatrix = globalData.projViewMatrix;

#pragma region Filter Renderable Meshes

		Frustum cameraFrustum = Frustum::Extract(globalData.projViewMatrix);

		// process mesh
		for (auto scene : scenes) {
			
			// TODO: Light processing
			// 就是frustum culling point light不在場景裡的不會process
			// process好後lightCount更新
			auto lightView = scene->getRegistry().view<TransformComponent, LightComponent>();
			for (auto [entity, transCom, lightCom] : lightView.each()) {
				switch (lightCom.type)
				{
				case LightType::Directional:
					// 無條件加入
					break;
				case LightType::Point:
					// 看radius有沒有在camera內
					break;
				case LightType::Spot:
					// 跟Point Light一樣
					break;
				default:
					break;
				}
			}
			
			auto sceneView = scene->getRegistry().view<
				TransformComponent,
				MeshComponent,
				MeshRendererComponent>();
			for (auto [entity, transformCom, meshCom, meshRendererCom] : sceneView.each()) {
				const auto& transform = transformCom.transform;
				const auto& mesh = meshCom.mesh;

				// meshRenderer的materials跟subMesh是一對一的
				PE_CORE_ASSERT(mesh->getSubMeshes().size() == meshRendererCom.materials.size(), "Wired mesh renderer materials doesn't match mesh submeshes");

				// TODO shadowmap mesh processing & culling
				// 使用culling後的light list cull各個light的mesh list

				// Frustum culling for meshes
				if (!cameraFrustum.isAABBInFrustum(meshCom.worldAABB))
					continue;

				if (mesh->getType() == MeshType::Static) {

					for (uint32_t subMeshIndex = 0; subMeshIndex < mesh->getSubMeshes().size(); subMeshIndex++) {
						auto material = meshRendererCom.materials[subMeshIndex];

						if (!material || !material->getBindingSet())
							continue;			// TODO: 改成null material之類的可以顯示

						m_meshRenderer.addEntity(
							material,
							mesh,
							subMeshIndex,
							transform);
					}
				}
				// TODO process skinned meshes
			}
		}

#pragma endregion
		// TODO split mesh renderer prepare render data to here
		// TODO 使用meshRenderer prepare完的renderData render forward plus depth texture
		// TODO compute light tiles using the filtered lights

		// TODO render shadow maps that are visible in camera viewport
		// 就是frustum culling

		m_meshRenderer.renderScene(sceneData);

		// TODO post processing

		// TODO 2d stuff rendering
	}

	void SceneRenderer::onBackBufferResized() {
		m_meshRenderer.onBackBufferResized();
	}
}
