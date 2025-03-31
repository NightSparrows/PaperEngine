#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void init() = 0;
		virtual void swapBuffers() = 0;

		virtual void beginFrame() = 0;
		virtual void endFrame() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}