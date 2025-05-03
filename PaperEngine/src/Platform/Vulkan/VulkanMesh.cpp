#include "VulkanMesh.h"

#include <PaperEngine/renderer/MeshData.h>

#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"


namespace PaperEngine {

	VulkanMesh::VulkanMesh()
	{
	}

	void VulkanMesh::load_mesh_data(const MeshData& meshData)
	{
		m_boneCount = meshData.boneCount;
		m_type = meshData.type;

		BufferSpecification bufferSpec;
		bufferSpec.setIsStorageBuffer(true);
		bufferSpec.setSize(static_cast<uint32_t>(meshData.basicVertexData.size()) * sizeof(MeshData::BasicVertexData));
		m_basicVertexBuffer = CreateRef<VulkanBuffer>(bufferSpec);

		if (m_type == MeshType::Animated) {
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
		if (m_type == MeshType::Animated) {
			cmd->writeBuffer(m_boneVertexBuffer, meshData.boneVertexData.data(), m_boneVertexBuffer->get_size());
		}
		cmd->writeBuffer(m_indexBuffer, meshData.indexData.data(), m_indexBuffer->get_size());
		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(std::static_pointer_cast<CommandBuffer>(cmd));

		for (const auto& subMesh : meshData.subMeshData) {
			m_subMeshes.emplace_back(subMesh.offset, subMesh.count);
		}
	}
	Ref<Mesh> VulkanMesh::clone()
	{
		Ref<VulkanMesh> newMesh = CreateRef<VulkanMesh>();
		newMesh->m_type = m_type;
		newMesh->m_boneCount = m_boneCount;
		newMesh->m_subMeshes = m_subMeshes;

		BufferSpecification bufferSpec;
		bufferSpec.setIsStorageBuffer(true);
		bufferSpec.setSize(static_cast<uint32_t>(m_basicVertexBuffer->get_size()));
		newMesh->m_basicVertexBuffer = CreateRef<VulkanBuffer>(bufferSpec);

		if (m_type == MeshType::Animated) {
			bufferSpec.setSize(static_cast<uint32_t>(m_boneVertexBuffer->get_size()));
			newMesh->m_boneVertexBuffer = CreateRef<VulkanBuffer>(bufferSpec);
		}

		bufferSpec.isIndexBuffer = true;
		bufferSpec.isStorageBuffer = false;
		bufferSpec.setSize(static_cast<uint32_t>(m_indexBuffer->get_size()));
		newMesh->m_indexBuffer = CreateRef<VulkanBuffer>(bufferSpec);


		VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();
		cmd->open();

		cmd->copyBuffer(m_basicVertexBuffer, newMesh->m_basicVertexBuffer, m_basicVertexBuffer->get_size(), 0, 0);
		if (m_type == MeshType::Animated) {
			cmd->copyBuffer(m_boneVertexBuffer, newMesh->m_boneVertexBuffer, m_boneVertexBuffer->get_size(), 0, 0);
		}
		cmd->copyBuffer(m_indexBuffer, newMesh->m_indexBuffer, m_indexBuffer->get_size(), 0, 0);

		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);

		return newMesh;
	}
	MeshType VulkanMesh::get_type() const
	{
		return m_type;
	}
}
