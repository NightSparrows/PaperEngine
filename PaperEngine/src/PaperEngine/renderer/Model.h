#pragma once

#include "Mesh.h"
#include "Material.h"

namespace PaperEngine {

	/// <summary>
	/// Loaded set of mesh and materials
	/// the materials is match the mesh submeshes
	/// </summary>
	struct Model {
		MeshHandle mesh;
		std::vector<MaterialHandle> materials;
	};

	typedef std::shared_ptr<Model> ModelHandle;
}