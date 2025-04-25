#include "SceneRenderer.h"

#include <Platform/Vulkan/VulkanSceneRenderer.h>

namespace PaperEngine {

	Ref<SceneRenderer> SceneRenderer::Create(const SceneRendererSpec& spec) {
		return CreateRef<VulkanSceneRenderer>(spec);
	}

}