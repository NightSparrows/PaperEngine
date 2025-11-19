#include "BindingSet.h"

#include <PaperEngine/core/Application.h>

namespace PaperEngine {

	BindingSet::BindingSet(IResource::Usage usage,
		nvrhi::BindingLayoutHandle layout, std::vector<nvrhi::BindingSetDesc> descs)
	{
		switch (usage)
		{
		case PaperEngine::IResource::Usage::Static:
		{
			m_storages.resize(1);
			m_storages[0].handle = Application::GetNVRHIDevice()->createBindingSet(descs[0], layout);
		}
			break;
		case PaperEngine::IResource::Usage::FrameStreaming:
		{
			m_storages.resize(Application::Get()->getGraphicsContext()->getMaxFrameInFlight());
			for (size_t i = 0; i < m_storages.size(); i++)
			{
				m_storages[i].handle = Application::GetNVRHIDevice()->createBindingSet(descs[i], layout);
			}
		}
			break;
		default:
			break;
		}
	}

	nvrhi::BindingSetHandle BindingSet::getCurrentHandle()
	{
		uint32_t index = 0;
		if (m_storages.size() > 1)
		{
			index = Application::Get()->getGraphicsContext()->getCurrentFrameInFlightIndex();
		}
		return m_storages[index].handle;
	}
}
