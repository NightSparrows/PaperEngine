#pragma once

#include <span>

#include <nvrhi/nvrhi.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

namespace PaperEngine {

	struct GlobalData {
		Ref<Camera> camera;					// 相機projection
		Transform cameraTransform;			// 相機transformation (view matrix)
		nvrhi::IBindingSet* globalSet;
		nvrhi::IFramebuffer* fb;
	};

	class IRenderer {
	public:

		virtual void renderScene(std::span<Ref<Scene>> scenes, const GlobalData& globalData) = 0;

	};

}
