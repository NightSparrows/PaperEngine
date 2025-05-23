#pragma once

#include <imgui.h>
#include <PaperEngine/renderer/Texture.h>

namespace PaperEngine {

	class ImGuiUtils {
	public:

		PE_API static ImTextureID GetImGuiTexture(TextureHandle texture);

		PE_API static void FreeImGuiTexture(ImTextureID texture);

	};

}
