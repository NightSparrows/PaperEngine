#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Mesh.h>
#include <PaperEngine/renderer/Buffer.h>
#include <PaperEngine/renderer/Material.h>

namespace PaperEngine {

	struct MeshComponent {
		MeshHandle mesh;

		// vector<Material> for materials it must be one by one to submeshes
		std::vector<MaterialHandle> materials;

		// bone transformations if mesh is animated (skinned)
		// the mesh component do not control how this buffer update data it just use it
		// other that control bone component will use this buffer to update its controlled bone transformations
		BufferHandle boneTransformBuffer;
	};

}
