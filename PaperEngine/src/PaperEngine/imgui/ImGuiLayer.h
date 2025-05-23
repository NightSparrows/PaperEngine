#pragma once

#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Layer.h>
#include <PaperEngine/renderer/Texture.h>

#include <imgui.h>

namespace PaperEngine {

	class ImGuiLayer : public Layer {
	public:

		virtual bool begin_frame() = 0;

		virtual void end_frame() = 0;

		virtual ImTextureID addTextureImpl(TextureHandle texture) = 0;



		static Ref<ImGuiLayer> Create();

		static std::shared_ptr<ImGuiLayer> GetInstance();

	protected:
		static std::weak_ptr<ImGuiLayer> s_instance;
	};

}
