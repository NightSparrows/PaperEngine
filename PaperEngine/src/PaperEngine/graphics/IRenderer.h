#pragma once
#pragma warning(push)
#pragma warning(disable : 4100)


#include <span>

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/BoundingVolume.h>
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

		virtual void processScene(Ref<Scene> scene, const Frustum& frustum) {};

		virtual void renderScene(nvrhi::ICommandList* cmd, const GlobalSceneData& globalData) = 0;

		// 將process的資料清空，準備下一frame使用
		virtual void endFrame() {}

		virtual void onViewportResized(uint32_t width, uint32_t height) {}

	};

}

#pragma warning(pop)