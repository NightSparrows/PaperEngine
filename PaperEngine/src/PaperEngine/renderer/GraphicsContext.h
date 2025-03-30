#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void init() = 0;
		virtual void swapBuffers() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};
}