﻿#pragma once

#include <cstdint>

#include "Texture.h"
#include "GraphicsPipeline.h"
#include "DescriptorSet.h"

namespace PaperEngine {

	struct MaterialSpec {
		GraphicsPipelineHandle graphicsPipeline;
	};

	class Material {
	public:
		virtual ~Material() = default;

		/// <summary>
		/// Update the material small data for its uniform buffer
		/// </summary>
		/// <param name="data"></param>
		/// <param name="size"></param>
		/// <param name="offset"></param>
		virtual void updateData(uint32_t binding, const void* data, size_t size, size_t offset) = 0;

		/// <summary>
		/// Update the texture in binding
		/// </summary>
		/// <param name="slot">
		/// </param>
		/// <param name="texture"></param>
		virtual void updateTexture(uint32_t binding, TextureHandle texture) = 0;

		virtual GraphicsPipelineHandle get_graphics_pipeline() const = 0;

		virtual DescriptorSetHandle getCurrentDescriptorSet() = 0;

		PE_API static Ref<Material> Create(const MaterialSpec& spec);

	protected:
		Material() = default;
	};

	typedef Ref<Material> MaterialHandle;
}
