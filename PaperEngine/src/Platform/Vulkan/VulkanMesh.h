#pragma once

#include <PaperEngine/renderer/Mesh.h>
#include "VulkanBuffer.h"
namespace PaperEngine {

	class VulkanMesh : public Mesh {
	public:
		VulkanMesh();

		void load_mesh_data(const MeshData& meshData) override;

		BufferHandle get_index_buffer() override { return m_indexBuffer; }

		BufferHandle get_basic_vertex_buffer() override { return m_basicVertexBuffer; }

		BufferHandle get_bone_vertex_buffer() override { return m_boneVertexBuffer; }

		std::vector<SubMesh>& get_sub_meshes() override { return m_subMeshes; }

		MeshType get_type() const override;

		uint32_t get_bone_count() const override { return m_boneCount; }

	protected:
		MeshType m_type{ MeshType::Static };

		Ref<VulkanBuffer> m_basicVertexBuffer;
		// that store the vertex bone information 
		// per vertex
		Ref<VulkanBuffer> m_boneVertexBuffer;
		uint32_t m_boneCount{ 0 };

		Ref<VulkanBuffer> m_indexBuffer;

		std::vector<SubMesh> m_subMeshes;
	};

}
