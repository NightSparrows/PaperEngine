#pragma once

#include <PaperEngine/core/Base.h>
#include "IResource.h"

#include <nvrhi/nvrhi.h>


namespace PaperEngine
{
	class GPUBuffer
	{
	public:
		struct Storage
		{
			nvrhi::BufferHandle handle;
			void* mapPtr;
		};
	public:
		GPUBuffer(ResourceUsage usage, nvrhi::BufferDesc desc);

		nvrhi::IBuffer* getHandle();

		void* getMapPtr();

		inline const std::vector<Storage>& getStorages() const { return m_storages; }

	private:
		std::vector<Storage> m_storages;
		ResourceUsage m_usage;
	};

	typedef Ref<GPUBuffer> GPUBufferHandle;

}
