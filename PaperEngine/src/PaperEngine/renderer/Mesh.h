#pragma once

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Buffer.h>

namespace PaperEngine {

	class MeshData;

	typedef enum MeshType : uint32_t {
		Static = 0,
		Animated = 1
	}MeshType;

	/// <summary>
	/// Contain only vertex and index data in GPU side
	/// no descriptor set, no material dependencies
	/// </summary>
	class Mesh {
	public:
		struct SubMesh {
			uint32_t offset;			// offset of index
			uint32_t count;				// index count
			uint32_t materialIndex;		// material index

			SubMesh(uint32_t off, uint32_t cou, uint32_t materialIdx) {
				offset = off;
				count = cou;
				materialIndex = materialIdx;
			}
		};
		virtual ~Mesh() = default;
		Mesh(const Mesh&) = delete;
		Mesh operator=(const Mesh&) = delete;

		virtual void load_mesh_data(const MeshData& meshData) = 0;

		/// <summary>
		/// Clone the mesh (different buffers)
		/// </summary>
		/// <returns>
		/// new mesh
		/// </returns>
		virtual Ref<Mesh> clone() = 0;

		virtual BufferHandle get_index_buffer() = 0;

		virtual BufferHandle get_basic_vertex_buffer() = 0;
		
		virtual BufferHandle get_bone_vertex_buffer() = 0;

		virtual std::vector<SubMesh>& get_sub_meshes() = 0;

		virtual MeshType get_type() const = 0;

		virtual uint32_t get_bone_count() const = 0;

		PE_API static Ref<Mesh> Create();

	protected:
		Mesh() = default;
	};

	typedef Ref<Mesh> MeshHandle;
}
