#pragma once

#include <nvrhi/nvrhi.h>
#include "IResource.h"

namespace PaperEngine
{
	class GPUBuffer
	{
	public:
		struct Storage
		{
			nvrhi::BufferHandle buffer;

			void* mappedPtr = nullptr;
		};

	public:
		GPUBuffer(IResource::Usage usage, nvrhi::BufferDesc bufferDesc);

		nvrhi::BufferHandle getCurrentHandle();

		/// <summary>
		/// Get the current frame in flight buffer storage memory pointer
		/// </summary>
		/// <returns></returns>
		void* mapBufferPtr();

		const std::vector<Storage>& getStorages() const { return m_storages; }

	private:
		std::vector<Storage> m_storages;
	};

	using GPUBufferHandle = Ref<GPUBuffer>;
}
