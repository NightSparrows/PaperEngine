#pragma once

#include "Mesh.h"
#include "Material.h"

namespace PaperEngine {

	/// <summary>
	/// 
	/// </summary>
	struct Model {
		MeshHandle mesh;
		std::vector<MaterialHandle> materials;
	};

}