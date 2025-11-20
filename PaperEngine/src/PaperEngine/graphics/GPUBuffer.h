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
		/// <summary>
		/// 
		/// </summary>
		/// <param name="usage">
		/// # Static
		///		Use in like the data in GPU will not be calculated or modify every frame
		/// for example, mesh vertex data
		/// # FrameStreaming
		///		Passing data from CPU to GPU every frame, auto enable cpu access write
		/// # FrameStatic
		///		the data need to be calculated in GPU every frame, but no need cpu access to it
		/// </param>
		/// <param name="desc"></param>
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
