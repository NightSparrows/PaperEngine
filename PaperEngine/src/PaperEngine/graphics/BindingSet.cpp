
#include <PaperEngine/core/Application.h>

#include "BindingSet.h"

namespace PaperEngine
{
	BindingSet::BindingSet(ResourceUsage usage, BindingLayoutHandle layout, const std::vector<nvrhi::BindingSetDesc>& descs)
		: m_usage(usage)
	{
		switch (usage)
		{
			using enum ResourceUsage;
		case Static:
		{
			m_storages.resize(1);
			m_storages[0].handle = Application::GetNVRHIDevice()->createBindingSet(descs[0], layout->handle);
		}
			break;
		case FrameStreaming:
		case FrameStatic:
		{
			const uint32_t max_frame_count = Application::Get()->getGraphicsContext()->getMaxFrameInFlight();
			PE_CORE_ASSERT(descs.size() == max_frame_count, "binding set descriptor is not equal to frame in flight count.");
			m_storages.resize(max_frame_count);
			for (uint32_t i = 0; i < max_frame_count; i++)
			{
				m_storages[i].handle = Application::GetNVRHIDevice()->createBindingSet(descs[i], layout->handle);
			}
		}
			break;
		default:
			break;
		}
	}

	nvrhi::IBindingSet* BindingSet::getHandle()
	{
		switch (m_usage)
		{
			using enum ResourceUsage;
		case Static:
			return m_storages[0].handle;
		case FrameStreaming:
		case FrameStatic:
			return m_storages[Application::Get()->getGraphicsContext()->getCurrentFrameIndex()].handle;
		default:
			return nullptr;
		}
	}

}
