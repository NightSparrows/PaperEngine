#pragma once

#include <PaperEngine/graphics/Mesh.h>
#include <PaperLoader/ModelData.h>

#include <filesystem>

namespace PaperEngine {

	/// <summary>
	/// To use this loader, you need to initialize the engine first
	/// </summary>
	class ModelLoader
	{
	public:

		static Ref<ModelData> LoadFromAssimp(const std::filesystem::path& filePath);
	};

}
