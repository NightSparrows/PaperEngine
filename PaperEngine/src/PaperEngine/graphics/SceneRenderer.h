#pragma once

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

#include <PaperEngine/graphics/MeshRenderer.h>
#include "ForwardPlusDepthRenderer.h"
#include "LightCullingPass.h"

namespace PaperEngine {

	struct GlobalDataI {
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projViewMatrix;
		glm::vec3 cameraPosition;
		float padding0;
		uint32_t directionalLightCount = 0;
		uint32_t pointLightCount = 0;
		uint32_t spotLightCount = 0;
		uint32_t numXSlices;
		uint32_t numYSlices;
		uint32_t numZSlices;
		float nearPlane;
		float farPlane;
	};

	/// <summary>
	/// 基本上用在場景裡的Camera
	/// 也能用在editor的虛擬Camera
	/// 使用Forward Plus Renderer
	/// 
	/// Process scenes (Not rendering)
	///		culling and process the light for the light culling pass
	///		culling and process the scene entities
	/// 
	/// rendering start
	/// clearing framebuffers
	/// 
	/// rendering preDepth pass
	/// 
	/// calculate lights culling pass
	/// 
	/// renderers that use the light culling data for rendering
	/// 
	/// 
	/// </summary>
	class SceneRenderer {
	public:
		PE_API SceneRenderer();
		PE_API ~SceneRenderer() = default;
		
		/// <summary>
		/// </summary>
		/// <param name="camera">
		/// 
		/// </param>
		/// <param name="transform">
		/// 相機位置
		/// </param>
		/// <param name="fb">
		/// 畫的framebuffer
		/// </param>
		PE_API void renderScene(std::span<Ref<Scene>> scenes, const Camera* camera, const Transform* transform, nvrhi::IFramebuffer* fb);

		PE_API void onBackBufferResized();

		PE_API MeshRenderer* getMeshRenderer() { return &m_meshRenderer; }

		PE_API LightCullingPass* getLightCullPass() { return &m_lightCullPass; }

	private:
		nvrhi::CommandListHandle m_cmd;

		BindingLayoutHandle m_globalLayout;
		nvrhi::BindingSetHandle m_globalSet;
		nvrhi::BufferHandle m_globalDataBuffer;

		// 先這樣
		MeshRenderer m_meshRenderer;
		ForwardPlusDepthRenderer m_forwardPlusDepthRenderer;
		LightCullingPass m_lightCullPass;

		// 等待整個場景render完的sync
		nvrhi::EventQueryHandle m_sceneRenderQuery;
	};

}
