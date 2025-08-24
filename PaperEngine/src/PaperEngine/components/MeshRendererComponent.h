#pragma once

#include <vector>

#include <PaperEngine/graphics/Material.h>

namespace PaperEngine {

	/// <summary>
	/// 一定要有Mesh component
	/// </summary>
	struct MeshRendererComponent {
		std::vector<Ref<Material>> materials;
	};

}
