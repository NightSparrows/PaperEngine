#pragma once

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

#include <PaperEngine/graphics/MeshRenderer.h>

namespace PaperEngine {

	/// <summary>
	/// 基本上用在場景裡的Camera
	/// 也能用在editor的虛擬Camera
	/// </summary>
	class SceneRenderer {
	public:
		PE_API SceneRenderer();
		PE_API ~SceneRenderer();
		
		/// <summary>
		/// Process場景
		/// 將會清除之前prepare的東西
		/// </summary>
		/// <param name="scene"></param>
		void prepareScenes(Scene* scene, uint32_t sceneCount = 1);

		/// <summary>
		/// 使用已經prepare好的Scene渲染到framebuffer上
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
		void renderScene(Camera* camera, Transform* transform, nvrhi::IFramebuffer* fb);

	private:
		nvrhi::CommandListHandle m_cmd;

		// 先這樣
		MeshRenderer m_meshRenderer;
	};

}
