
#include <PaperEngine/core/Base.h>
#include <PaperEngine/core/Application.h>

#include "Mesh.h"


namespace PaperEngine {

	void Mesh::loadRawData(const void* data, uint32_t size, uint32_t indexOffset)
	{
		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_indexBufferOffset = indexOffset;

		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(size)
			.setIsVertexBuffer(true)
			.setIsIndexBuffer(true);
		m_buffer = device->createBuffer(bufferDesc);

		auto cmd = device->createCommandList();
		cmd->open();
		cmd->writeBuffer(m_buffer, data, size);
		cmd->close();
		Application::Get()->getGraphicsContext()->getNVRhiDevice()->executeCommandList(cmd, nvrhi::CommandQueue::Copy);
	}

	void Mesh::loadRawDataAsync(nvrhi::CommandListHandle cmd, const void* data, uint32_t size, uint32_t indexOffset)
	{
		PE_CORE_ASSERT(cmd, "Mesh::loadRawDataAsync: Command list is null");

		auto device = Application::Get()->getGraphicsContext()->getNVRhiDevice();

		m_indexBufferOffset = indexOffset;

		auto bufferDesc = nvrhi::BufferDesc()
			.setByteSize(size)
			.setIsVertexBuffer(true)
			.setIsIndexBuffer(true);
		m_buffer = device->createBuffer(bufferDesc);

		cmd->writeBuffer(m_buffer, data, size);
	}

	void Mesh::bindSubMesh(nvrhi::GraphicsState& state, nvrhi::DrawArguments& drawArgs, uint32_t subMeshIndex) const
	{
		PE_CORE_ASSERT(subMeshIndex < m_subMeshes.size(), "Mesh::bindSubMesh: subMeshIndex out of range");
		const SubMeshInfo& subMesh = m_subMeshes[subMeshIndex];
		
		state.indexBuffer.buffer = m_buffer;
		state.indexBuffer.format = nvrhi::Format::R32_UINT; // Assuming 32-bit indices, can be changed if needed
		state.indexBuffer.offset = m_indexBufferOffset;
		
		drawArgs.vertexCount = subMesh.indicesCount;
		drawArgs.startIndexLocation = subMesh.indicesOffset;
		drawArgs.startVertexLocation = 0; // Assuming no vertex offset, can be modified if needed
	}

}
