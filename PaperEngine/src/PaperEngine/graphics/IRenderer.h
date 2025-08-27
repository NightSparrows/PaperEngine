#pragma once

#include <span>

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

namespace PaperEngine {

	struct GlobalSceneData {
		const Camera* camera;					// 相機projection
		const Transform* cameraTransform;			// 相機transformation (view matrix)
		glm::mat4 projViewMatrix;
		nvrhi::IBindingSet* globalSet;
		nvrhi::IFramebuffer* fb;
	};

	class IRenderer {
	public:

		virtual void renderScene(const GlobalSceneData& globalData) = 0;

		virtual void onViewportResized(uint32_t width, uint32_t height) {}

	};

}
