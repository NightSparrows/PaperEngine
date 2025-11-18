#include "GPUBuffer.h"


#include <PaperEngine/core/Application.h>

namespace PaperEngine
{

	GPUBuffer::GPUBuffer(IResource::Usage usage, nvrhi::BufferDesc bufferDesc)
	{
		switch (usage)
		{
		using enum PaperEngine::IResource::Usage;
		case Static:
		{
			m_storages.resize(1);
			m_storages[0].buffer = Application::GetNVRHIDevice()->createBuffer(bufferDesc);
		}
			break;
		case FrameStreaming:
		{
			bufferDesc.cpuAccess = nvrhi::CpuAccessMode::Write;
			m_storages.resize(Application::Get()->getGraphicsContext()->getMaxFrameInFlight());
			for (auto& storage : m_storages)
			{
				storage.buffer = Application::GetNVRHIDevice()->createBuffer(bufferDesc);
				storage.mappedPtr = Application::GetNVRHIDevice()->mapBuffer(storage.buffer, nvrhi::CpuAccessMode::Write);
			}
		}
			break;
		default:
			break;
		}
	}

	nvrhi::BufferHandle GPUBuffer::getCurrentHandle()
	{
		uint32_t index = 0;
		if (m_storages.size() > 1)
		{
			index = Application::Get()->getGraphicsContext()->getCurrentFrameInFlightIndex();
		}
		return m_storages[index].buffer;
	}

	void* GPUBuffer::mapBufferPtr()
	{
		PE_CORE_ASSERT(m_storages.size() > 1, "Only FrameStreaming buffer can map buffer ptr!");


		return m_storages[Application::Get()->getGraphicsContext()->getCurrentFrameInFlightIndex()].mappedPtr;
	}

} // namespace PaperEngine
