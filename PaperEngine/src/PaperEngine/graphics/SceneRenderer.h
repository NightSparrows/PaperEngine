#pragma once

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

#include <PaperEngine/graphics/MeshRenderer.h>

namespace PaperEngine {

	struct GlobalDataI {
		glm::mat4 projectionMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projViewMatrix;
		glm::vec3 cameraPosition;
		float padding0;
	};

	/// <summary>
	/// 基本上用在場景裡的Camera
	/// 也能用在editor的虛擬Camera
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

	private:
		nvrhi::CommandListHandle m_cmd;

		BindingLayoutHandle m_globalLayout;
		nvrhi::BindingSetHandle m_globalSet;
		nvrhi::BufferHandle m_globalDataBuffer;

		// 先這樣
		MeshRenderer m_meshRenderer;
	};

}
