#pragma once

#include <PaperEngine/core/Layer.h>

#include <imgui.h>

namespace PaperEngine {

	class ImGuiLayer : public Layer {
	public:

		static Ref<ImGuiLayer> Create();

	};

}
