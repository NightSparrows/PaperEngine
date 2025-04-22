#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Mesh.h>
#include <PaperEngine/renderer/Material.h>

namespace PaperEngine {

	struct MeshComponent {
		MeshHandle mesh;

		/// <summary>
		/// this must be bound to sub mesh one by one
		/// </summary>
		std::vector<MaterialHandle> materials;

		nvrhi::BindingSetHandle instanceBindingSet{ nullptr };
	};

}
