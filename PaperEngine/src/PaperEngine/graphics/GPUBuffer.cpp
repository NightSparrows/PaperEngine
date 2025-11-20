
#include <PaperEngine/core/Application.h>

#include "GPUBuffer.h"

namespace PaperEngine {

	GPUBuffer::GPUBuffer(ResourceUsage usage, nvrhi::BufferDesc desc)
		: m_usage(usage)
	{
		switch (usage)
		{
			using enum PaperEngine::ResourceUsage;
		case Static:
		{
			m_storages.resize(1);

			m_storages[0].handle = Application::GetNVRHIDevice()->createBuffer(desc);
		}
			break;
		case FrameStreaming:
		{
			desc.cpuAccess = nvrhi::CpuAccessMode::Write;
			m_storages.resize(Application::Get()->getGraphicsContext()->getMaxFrameInFlight());
			for (auto& storage : m_storages)
			{
				storage.handle = Application::GetNVRHIDevice()->createBuffer(desc);
				storage.mapPtr = Application::GetNVRHIDevice()->mapBuffer(storage.handle, nvrhi::CpuAccessMode::Write);
			}
		}
			break;
		case FrameStatic:
		{
			m_storages.resize(Application::Get()->getGraphicsContext()->getMaxFrameInFlight());
			for (auto& storage : m_storages)
			{
				storage.handle = Application::GetNVRHIDevice()->createBuffer(desc);
			}
		}
			break;
		default:
			break;
		}
	}

	nvrhi::IBuffer* GPUBuffer::getHandle()
	{
		switch (m_usage)
		{
			using enum PaperEngine::ResourceUsage;
		case Static:
			return m_storages[0].handle;
		case FrameStreaming:
		case FrameStatic:
			return m_storages[Application::Get()->getGraphicsContext()->getCurrentFrameIndex()].handle;
		default:
			return nullptr;
		}
	}
	void* GPUBuffer::getMapPtr()
	{
		PE_CORE_ASSERT(m_usage == ResourceUsage::FrameStreaming, "cannot get map pointer, buffer is not frame streaming.");
		return m_storages[Application::Get()->getGraphicsContext()->getCurrentFrameIndex()].mapPtr;
	}
}
