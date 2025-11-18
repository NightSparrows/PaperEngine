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

	SceneRenderer::SceneRenderer()
	{
		m_lightCullPass.init();

		// 全域data (constantBuffer Slot 0 : set = 0)
		nvrhi::BufferDesc globalDataBufferDesc;
		globalDataBufferDesc
			.setInitialState(nvrhi::ResourceStates::ConstantBuffer)
			.setKeepInitialState(true)
			.setByteSize(sizeof(GlobalDataI))
			.setIsConstantBuffer(true);
		m_globalDataBuffer = CreateRef<GPUBuffer>(IResource::Usage::FrameStreaming, globalDataBufferDesc);

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

		std::vector<nvrhi::BindingSetDesc> globalSetDescs;
		for (uint32_t i = 0; i < Application::Get()->getGraphicsContext()->getMaxFrameInFlight(); i++)
		{
			nvrhi::BindingSetDesc globalSetDesc;
			globalSetDesc.addItem(nvrhi::BindingSetItem::ConstantBuffer(0, m_globalDataBuffer->getStorages()[i].buffer));
			globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_lightCullPass.getDirectionalLightBuffer()));
			globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(1, m_lightCullPass.getPointLightBuffer()));
			globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(2, m_lightCullPass.getPointLightCullData().globalLightIndicesBuffer));
			globalSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(3, m_lightCullPass.getPointLightCullData().clusterRangesBuffer));
			globalSetDescs.push_back(globalSetDesc);
		}
		m_globalSet = CreateRef<BindingSet>(IResource::Usage::FrameStreaming, m_globalLayout->handle, globalSetDescs);

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
		sceneData.globalSet = m_globalSet->getCurrentHandle();
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
				auto lightView = scene->getRegistry().group<LightComponent>(entt::get<TransformComponent>);
				for (auto [entity, lightCom, transCom] : lightView.each()) {
					m_lightCullPass.processLight(transCom.transform, lightCom);
				}

				m_meshRenderer.processScene(scene, cameraFrustum);
				// TODO process skinned meshes
			}
		}

#pragma endregion
		globalData.directionalLightCount = m_lightCullPass.getDirectionalLightCount();
		// 不代表會全部Process
		globalData.pointLightCount = m_lightCullPass.getPointLightCount();

		auto main_cmd = Application::Get()->getGraphicsContext()->getCurrentFrameCommandList();

		//m_cmd->open();
		auto swapchain_texture = fb->getDesc().colorAttachments[0].texture;
		auto depth_texture = fb->getDesc().depthAttachment.texture;


		// NVRHI有問題，他無法使用track一個一開始undefined layout導致Vulkan validation一直報錯
		main_cmd->beginTrackingTextureState(swapchain_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::Unknown);
		main_cmd->beginTrackingTextureState(depth_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::Unknown);

		main_cmd->setTextureState(swapchain_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::CopyDest);
		main_cmd->setTextureState(depth_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::CopyDest);

		main_cmd->commitBarriers();

		main_cmd->clearTextureFloat(swapchain_texture, nvrhi::AllSubresources, nvrhi::Color(0.f, 0.f, 0.f, 0.f));
		main_cmd->clearDepthStencilTexture(depth_texture, nvrhi::AllSubresources, true, 1.f, false, 0);

		main_cmd->writeBuffer(m_globalDataBuffer->getCurrentHandle(), &globalData, sizeof(globalData));

		// Render PreDepth Pass
		//m_forwardPlusDepthRenderer.renderScene(sceneData);
		// compute light tiles using the filtered lights and (TODO predepth texture)
		m_lightCullPass.calculatePass(main_cmd);

		// TODO render shadow maps that are visible in camera viewport

		m_meshRenderer.renderScene(main_cmd, sceneData);

		// TODO post processing

		// TODO wait for 3d finish rendering (becuase NVRHI doesn't have the command buffer hazal between command buffers
		// in GPU sides only having the cpu-gpu block api
		// TODO 2d stuff rendering

		//m_cmd->close();
		//Application::GetNVRHIDevice()->executeCommandList(m_cmd);

		// 清除process data
		m_meshRenderer.endFrame();
	}

	void SceneRenderer::onBackBufferResized() {


	}
}
