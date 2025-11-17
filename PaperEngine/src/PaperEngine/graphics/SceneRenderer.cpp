#include "SceneRenderer.h"

#include <PaperEngine/core/Application.h>

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/MeshComponent.h>
#include <PaperEngine/components/MeshRendererComponent.h>
#include <PaperEngine/components/LightComponent.h>
#include <PaperEngine/debug/Instrumentor.h>

#include <PaperEngine/utils/BoundingVolume.h>

#include <nvrhi/utils.h>

#include <execution>

namespace PaperEngine {

	SceneRenderer::SceneRenderer() :
		m_threadPool(8)
	{
		m_lightCullPass.init();

		m_sceneRenderQuery = Application::GetNVRHIDevice()->createEventQuery();
		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		// 全域data (constantBuffer Slot 0 : set = 0)
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
			.addItem(nvrhi::BindingLayoutItem::ConstantBuffer(0))
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0))		// directional Light buffer
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(1))		// point Light buffer
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(2))		// light indices buffer
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(3));	// cluster ranges buffer
		m_globalLayout =
			Application::GetResourceManager()->create<BindingLayout>("SceneRenderer_globalLayout",
				Application::GetNVRHIDevice()->createBindingLayout(globalLayoutDesc));

		nvrhi::BindingSetDesc globalSetDesc;
		globalSetDesc.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_globalDataBuffer));
		globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_lightCullPass.getDirectionalLightBuffer()));
		globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(1, m_lightCullPass.getPointLightBuffer()));
		globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(2, m_lightCullPass.getPointLightCullData().globalLightIndicesBuffer));
		globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(3, m_lightCullPass.getPointLightCullData().clusterRangesBuffer));
		m_globalSet = Application::GetNVRHIDevice()->createBindingSet(globalSetDesc, m_globalLayout->handle);

		m_forwardPlusDepthRenderer.init();
	}

	void SceneRenderer::renderScene(std::span<Ref<Scene>> scenes, const Camera* camera, const Transform* transform, nvrhi::IFramebuffer* fb)
	{
		PE_PROFILE_FUNCTION();

		// prepare processing
		m_lightCullPass.beginPass();

		// Global Data in GPU Buffer
		GlobalDataI globalData{};
		globalData.projectionMatrix = camera->getProjectionMatrix();
		globalData.viewMatrix = glm::inverse(transform->matrix());
		globalData.projViewMatrix = globalData.projectionMatrix * globalData.viewMatrix;
		globalData.cameraPosition = transform->getPosition();
		globalData.numXSlices = m_lightCullPass.getNumberOfXSlices();
		globalData.numYSlices = m_lightCullPass.getNumberOfYSlices();
		globalData.numZSlices = m_lightCullPass.getNumberOfZSlices();
		globalData.nearPlane = camera->getNearPlane();
		globalData.farPlane = camera->getFarPlane();

		GlobalSceneData sceneData;
		sceneData.camera = camera;
		sceneData.cameraTransform = transform;
		sceneData.fb = fb;
		sceneData.globalSet = m_globalSet;
		sceneData.projViewMatrix = globalData.projViewMatrix;

#pragma region Filter Renderable Meshes

		Frustum cameraFrustum = Frustum::Extract(globalData.projViewMatrix);
		m_lightCullPass.setCamera(*camera, globalData.viewMatrix, cameraFrustum);

		{
			PE_PROFILE_SCOPE("Process scene to renderer");

			// Process every scene
			for (auto scene : scenes) {

				// Light processing
				// 就是frustum culling point light不在場景裡的不會process
				// process好後lightCount更新
				auto lightView = scene->getRegistry().view<TransformComponent, LightComponent>();
				for (auto [entity, transCom, lightCom] : lightView.each()) {
					m_lightCullPass.processLight(transCom.transform, lightCom);
				}

				{
					PE_PROFILE_SCOPE("Mesh renderer processing");

					const auto scene_group = scene->getRegistry().group<MeshComponent>(entt::get<TransformComponent, MeshRendererComponent>);
#define SCENE_RENDERER_USE_MULTITHREADING
#ifdef SCENE_RENDERER_USE_MULTITHREADING

					{
						// Multithreaded processing
						auto group_start = scene_group.begin();
						auto group_end = scene_group.end();

						size_t thread_count = std::thread::hardware_concurrency();
						size_t chunk_size = (scene_group.size() + thread_count) / thread_count;
						//PE_CORE_TRACE("Process chunk size: {}", chunk_size);	

						std::vector<std::future<void>> futures(thread_count);

						for (size_t i = 0; i < thread_count; i++)
						{
							PE_PROFILE_SCOPE("Worker thread dispatcher for loop.");
							auto start_it = group_start;
							std::advance(group_start, chunk_size);
							futures[i] = m_threadPool.enqueue([&, group_start, group_end, thread_count, start_it, i]()
								{
									PE_PROFILE_SCOPE("Worker thread process mesh renderers");
									auto end_it = (i == thread_count - 1) ? group_end : group_start;
									for (auto it = start_it; it != end_it; ++it)
									{
										auto entity = *it;
										const auto& meshCom = scene_group.get<MeshComponent>(entity);
										const auto& meshRendererCom = scene_group.get<MeshRendererComponent>(entity);
										const auto mesh = meshCom.mesh;
										const auto& transform = scene_group.get<TransformComponent>(entity).transform;
										// Frustum culling for meshes
										if (!cameraFrustum.isIntersect(meshCom.worldAABB))
											continue;
										if (mesh->getType() != MeshType::Static)
											continue;
										// meshRenderer的materials跟subMesh是一對一的
										PE_CORE_ASSERT(mesh->getSubMeshes().size() == meshRendererCom.materials.size(), "Wired mesh renderer materials doesn't match mesh submeshes");
										m_forwardPlusDepthRenderer.addEntity(mesh, transform);
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
								});
						}

						for (auto& future : futures)
						{
							future.get();
						}
					}
#else

					{
						// single thread processing (for reference)

						scene_group.each([&](auto, auto& meshCom, auto& transformCom, auto& meshRendererCom)
							{
								const auto mesh = meshCom.mesh;
								const auto& transform = transformCom.transform;

								// Frustum culling for meshes
								if (!cameraFrustum.isIntersect(meshCom.worldAABB))
									return;

								if (mesh->getType() != MeshType::Static)
									return;

								// meshRenderer的materials跟subMesh是一對一的
								PE_CORE_ASSERT(mesh->getSubMeshes().size() == meshRendererCom.materials.size(), "Wired mesh renderer materials doesn't match mesh submeshes");

								m_forwardPlusDepthRenderer.addEntity(mesh, transform);

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
							});
					}

#endif // SCENE_RENDERER_USE_MULTITHREADING

				}

				// TODO process skinned meshes
			}
		}

#pragma endregion
		globalData.directionalLightCount = m_lightCullPass.getDirectionalLightCount();
		// 不代表會全部Process
		globalData.pointLightCount = m_lightCullPass.getPointLightCount();

		m_cmd->open();

		nvrhi::utils::ClearColorAttachment(m_cmd, fb, 0, nvrhi::Color(0));
		nvrhi::utils::ClearDepthStencilAttachment(m_cmd, fb, 1.f, 0);

		m_cmd->writeBuffer(m_globalDataBuffer, &globalData, sizeof(globalData));

		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);

		// Render PreDepth Pass
		m_forwardPlusDepthRenderer.renderScene(sceneData);
		// compute light tiles using the filtered lights and (TODO predepth texture)
		m_lightCullPass.calculatePass();

		Application::GetNVRHIDevice()->resetEventQuery(m_sceneRenderQuery);
		// TODO render shadow maps that are visible in camera viewport

		m_meshRenderer.renderScene(sceneData);

		// TODO post processing

		// TODO wait for 3d finish rendering (becuase NVRHI doesn't have the command buffer hazal between command buffers
		// in GPU sides only having the cpu-gpu block api
		Application::GetNVRHIDevice()->setEventQuery(m_sceneRenderQuery, nvrhi::CommandQueue::Graphics);
		Application::GetNVRHIDevice()->waitEventQuery(m_sceneRenderQuery);

		// TODO 2d stuff rendering
		
		//Application::GetNVRHIDevice()->waitForIdle();
	}

	void SceneRenderer::onBackBufferResized() {


	}
}
