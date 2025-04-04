#pragma once

#include <PaperEngine/core/Layer.h>

namespace PaperEngine {

	class ImGuiLayerImpl;

	/// <summary>
	/// basic setup imgui for you
	/// do not export to user since this is 
	/// a native application layer
	/// </summary>
	class ImGuiLayer : public Layer {
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void on_attach() override;
		void on_detach() override;

		// need to be inside the command render pass
		void begin_frame();
		void end_frame();

	protected:
		ImGuiLayerImpl* m_impl{ nullptr };
	};

}
