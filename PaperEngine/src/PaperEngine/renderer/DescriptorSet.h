#pragma once

#include <PaperEngine/renderer/Buffer.h>
#include <PaperEngine/renderer/Texture.h>

#include "DescriptorSetLayout.h"

namespace PaperEngine {

	class DescriptorSet {
	public:
		virtual ~DescriptorSet() = default;

		virtual void bindBuffer(uint32_t binding, DescriptorType type, BufferHandle buffer, size_t offset, size_t range) = 0;

		virtual void bindTextures(uint32_t binding, uint32_t textureCount, TextureHandle* textures) = 0;

		static Ref<DescriptorSet> Allocate(DescriptorSetLayoutHandle setLayout);

	protected:
		DescriptorSet() = default;
	};

	typedef Ref<DescriptorSet> DescriptorSetHandle;
}
