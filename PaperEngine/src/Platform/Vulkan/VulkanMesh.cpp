#include "VulkanMesh.h"


#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"

namespace PaperEngine {

	VulkanMesh::VulkanMesh()
	{
	}

	void VulkanMesh::load_mesh_data(const MeshData& meshData)
	{
		m_type = meshData.type;

		BufferSpecification bufferSpec;
		bufferSpec.setIsStorageBuffer(true);
		bufferSpec.setSize(static_cast<uint32_t>(meshData.basicVertexData.size()) * sizeof(MeshData::BasicVertexData));
		m_basicVertexBuffer = CreateRef<VulkanBuffer>(bufferSpec);

		if (m_type == MeshData::Animated) {
			bufferSpec.setSize(static_cast<uint32_t>(meshData.boneVertexData.size()) * sizeof(MeshData::BoneVertexData));
			m_boneVertexBuffer = CreateRef<VulkanBuffer>(bufferSpec);
		}

		bufferSpec.isIndexBuffer = true;
		bufferSpec.isStorageBuffer = false;
		bufferSpec.setSize(static_cast<uint32_t>(meshData.indexData.size()) * sizeof(uint32_t));
		m_indexBuffer = CreateRef<VulkanBuffer>(bufferSpec);

		VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();
		cmd->open();
		cmd->writeBuffer(m_basicVertexBuffer, meshData.basicVertexData.data(), m_basicVertexBuffer->get_size());
		if (m_type == MeshData::Animated) {
			cmd->writeBuffer(m_boneVertexBuffer, meshData.boneVertexData.data(), m_boneVertexBuffer->get_size());
		}
		cmd->writeBuffer(m_indexBuffer, meshData.indexData.data(), m_indexBuffer->get_size());
		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(std::static_pointer_cast<CommandBuffer>(cmd));
	}
}
