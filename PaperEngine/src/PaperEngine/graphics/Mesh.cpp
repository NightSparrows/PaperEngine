
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
		Application::Get()->getGraphicsContext()->getNVRhiDevice()->executeCommandList(cmd);
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

}
