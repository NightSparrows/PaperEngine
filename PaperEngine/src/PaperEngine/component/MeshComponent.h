#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Mesh.h>
#include <PaperEngine/renderer/Buffer.h>
#include <PaperEngine/renderer/Material.h>
#include <PaperEngine/renderer/DescriptorSet.h>

namespace PaperEngine {

	struct MeshComponent {
		MeshHandle mesh;

		// vector<Material> for materials it must be one by one to submeshes
		std::vector<MaterialHandle> materials;

		// Control by the mesh renderer do not modify this
		// there frame in flight instance set
		mutable std::vector<DescriptorSetHandle> instanceSets;

		/// <summary>
		/// Control by the mesh renderer do not modify this
		/// </summary>
		mutable std::vector<BufferHandle> instanceInfoBuffer;

		// Control by the mesh renderer do not modify this
		// bone transformations if mesh is animated (skinned)
		// the mesh component do not control how this buffer update data it just use it
		// other that control bone component will use this buffer to update its controlled bone transformations
		mutable std::vector<BufferHandle> boneTransformBuffer;
	};

}
