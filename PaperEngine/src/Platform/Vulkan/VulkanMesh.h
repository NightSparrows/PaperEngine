#pragma once

#include <PaperEngine/renderer/Mesh.h>
#include "VulkanBuffer.h"
namespace PaperEngine {

	class VulkanMesh : public Mesh {
	public:
		VulkanMesh();

		void load_mesh_data(const MeshData& meshData) override;

		BufferHandle get_index_buffer() override { return m_indexBuffer; }

	protected:
		MeshData::MeshType m_type{ MeshData::MeshType::Static };

		Ref<VulkanBuffer> m_basicVertexBuffer;
		// that store the vertex bone information 
		// per vertex
		Ref<VulkanBuffer> m_boneVertexBuffer;
		Ref<VulkanBuffer> m_indexBuffer;
	};

}
