#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	struct BufferSpecification
	{
		uint32_t size = 0;
		bool isVertexBuffer = false;
		bool isStorageBuffer = false;
		bool isIndexBuffer = false;
		bool isUniformBuffer = false;

		BufferSpecification& setIsVertexBuffer(bool isVertexBuffer)
		{
			this->isVertexBuffer = isVertexBuffer;
			return *this;
		}

		BufferSpecification& setIsStorageBuffer(bool isStorageBuffer)
		{
			this->isStorageBuffer = isStorageBuffer;
			return *this;
		}

		BufferSpecification& setIsIndexBuffer(bool isIndexBuffer)
		{
			this->isIndexBuffer = isIndexBuffer;
			return *this;
		}

		BufferSpecification& setIsUniformBuffer(bool isUniformBuffer)
		{
			this->isUniformBuffer = isUniformBuffer;
			return *this;
		}

		BufferSpecification& setSize(uint32_t size) {
			this->size = size;
			return *this;
		}
	};

	class Buffer
	{
	public:
		virtual ~Buffer() = default;

		virtual size_t get_size() const = 0;

		static Ref<Buffer> Create(const BufferSpecification& spec);

	protected:
		Buffer() = default;
	};

	typedef Ref<Buffer> BufferHandle;
}
