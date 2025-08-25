#include "SceneRenderer.h"

#include <PaperEngine/core/Application.h>

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
		// 
		GlobalDataI globalData{};
		globalData.projectionMatrix = camera->getProjectionMatrix();
		globalData.viewMatrix = glm::inverse(transform->matrix());
		globalData.projViewMatrix = globalData.projectionMatrix * globalData.viewMatrix;
		globalData.cameraPosition = transform->getPosition();
		m_cmd->open();

		m_cmd->writeBuffer(m_globalDataBuffer, &globalData, sizeof(globalData));

		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);

		GlobalSceneData sceneData;
		sceneData.camera = camera;
		sceneData.cameraTransform = transform;
		sceneData.fb = fb;
		sceneData.globalSet = m_globalSet;

		m_meshRenderer.renderScene(scenes, sceneData);
	}

}
