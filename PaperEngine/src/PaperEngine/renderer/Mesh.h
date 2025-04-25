#pragma once

#include <PaperEngine/core/Base.h>

#include "MeshData.h"
#include <PaperEngine/renderer/Buffer.h>

namespace PaperEngine {

	/// <summary>
	/// Contain only vertex and index data
	/// no descriptor set
	/// </summary>
	class Mesh {
	public:
		struct SubMesh {
			uint32_t offset;			// offset of index
			uint32_t count;				// index count
		};

		virtual ~Mesh() = default;

		virtual void load_mesh_data(const MeshData& meshData) = 0;

		virtual BufferHandle get_index_buffer() = 0;

		PE_API static Ref<Mesh> Create();

	protected:
		Mesh() = default;
	};

	typedef Ref<Mesh> MeshHandle;
}
