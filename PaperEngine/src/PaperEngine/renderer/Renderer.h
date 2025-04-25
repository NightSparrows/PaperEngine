#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/scene/Scene.h>

#include <PaperEngine/renderer/CommandBuffer.h>

namespace PaperEngine {

	class Renderer {
	public:

		virtual void prepareScene(const Scene& scene) = 0;

		virtual void render(CommandBufferHandle cmd) = 0;

	};

	typedef Ref<Renderer> RendererHandle;
}
