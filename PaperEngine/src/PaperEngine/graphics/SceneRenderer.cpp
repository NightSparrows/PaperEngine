#include "SceneRenderer.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {

	SceneRenderer::SceneRenderer()
	{
		m_cmd = Application::GetNVRHIDevice()->createCommandList();
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	void SceneRenderer::prepareScenes(Scene* scene, uint32_t sceneCount)
	{
	}

	void SceneRenderer::renderScene(Camera* camera, Transform* transform, nvrhi::IFramebuffer* fb)
	{
	}

}
