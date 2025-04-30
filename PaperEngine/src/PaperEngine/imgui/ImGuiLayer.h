#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Layer.h>

namespace PaperEngine {

	class ImGuiLayer : public Layer {
	public:

		virtual bool begin_frame() = 0;

		virtual void end_frame() = 0;

		static Ref<ImGuiLayer> Create();

	protected:

	};

}
