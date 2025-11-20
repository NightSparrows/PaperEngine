#pragma once

#include <PaperEngine/core/Base.h>
#include "IResource.h"
#include "BindingLayout.h"

#include <nvrhi/nvrhi.h>


namespace PaperEngine
{
	class BindingSet
	{
	public:
		struct Storage
		{
			nvrhi::BindingSetHandle handle;
		};
	public:
		BindingSet(ResourceUsage usage, BindingLayoutHandle layout, const std::vector<nvrhi::BindingSetDesc>& descs);

		nvrhi::IBindingSet* getHandle();

		inline const std::vector<Storage>& getStorages() const { return m_storages; }

	private:
		std::vector<Storage> m_storages;
		ResourceUsage m_usage;
	};

	typedef Ref<BindingSet> BindingSetHandle;

}
