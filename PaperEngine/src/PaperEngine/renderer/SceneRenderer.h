#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/scene/Scene.h>
#include <PaperEngine/renderer/Renderer.h>

namespace PaperEngine {

	struct SceneRendererSpec {
		uint32_t width, height;
	};

	class SceneRenderer {
	public:
		// the global uniform buffer (in binding 0 of global set) of this scene renderer
		struct GlobalUniformBufferStruct {
			glm::mat4 projectionMatrix;
			glm::mat4 viewMatrix;
		};

		virtual ~SceneRenderer() = default;
		SceneRenderer(const SceneRenderer&) = delete;
		SceneRenderer& operator=(const SceneRenderer&) = delete;

		virtual void resize(uint32_t width, uint32_t height) = 0;

		virtual void renderScene(const Scene& scene) = 0;

		virtual void addRenderer(RendererHandle renderer) = 0;

		PE_API static Ref<SceneRenderer> Create(const SceneRendererSpec& spec);

	protected:
		SceneRenderer() = default;
	};

}
