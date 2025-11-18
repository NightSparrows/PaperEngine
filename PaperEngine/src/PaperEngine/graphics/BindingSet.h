#pragma once

#include <nvrhi/nvrhi.h>
#include "IResource.h"

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
		BindingSet(
			IResource::Usage usage, 
			nvrhi::BindingLayoutHandle layout,
			std::vector<nvrhi::BindingSetDesc> descs);

		nvrhi::BindingSetHandle getCurrentHandle();

	private:
		std::vector<Storage> m_storages;
	};

	using BindingSetHandle = Ref<BindingSet>;
}
