#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/scene/Scene.h>

#include <PaperEngine/renderer/CommandBuffer.h>
#include <PaperEngine/renderer/DescriptorSet.h>

namespace PaperEngine {

	class Renderer {
	public:

		virtual void prepareScene(const Scene& scene) = 0;

		virtual void render(CommandBufferHandle cmd, DescriptorSetHandle globalSet) = 0;

	};

	typedef Ref<Renderer> RendererHandle;
}
