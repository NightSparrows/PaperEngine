#pragma once

#include <PaperEngine/core/Layer.h>

#include <PaperEngine/imgui/ImGuiInclude.h>

namespace PaperEngine {

	class ImGuiLayer : public Layer {
	public:

		static Ref<ImGuiLayer> Create();

	};

}
