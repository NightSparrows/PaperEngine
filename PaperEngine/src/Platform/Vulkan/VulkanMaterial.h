#pragma once

#include <PaperEngine/renderer/Material.h>
#include "VulkanGraphicsPipeline.h"

#include <vk_mem_alloc.h>

#include "VulkanDescriptorSet.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"

namespace PaperEngine {

	class VulkanMaterial : public Material {
	public:
		enum BindingType {
			Uniformbuffer,
			Texture
		};

		struct MaterialCPUData {
			BindingType type;
			std::vector<char> buffer;
			VulkanTextureHandle texture;
		};

		struct MaterialBinding {
			Ref<VulkanBuffer> buffer;
			Ref<VulkanTexture> texture;
		};
		struct MaterialDataFrame {
			std::unordered_map<uint32_t, MaterialBinding> bindings;
			Ref<VulkanDescriptorSet> set;
			// only that dirty
			std::unordered_map<uint32_t, bool> dirtySymbol;
		};

		VulkanMaterial(const MaterialSpec& spec);
		~VulkanMaterial();

		void updateData(uint32_t binding, const void* data, size_t size, size_t offset) override;

		void updateTexture(uint32_t binding, TextureHandle texture) override;

		GraphicsPipelineHandle get_graphics_pipeline() const override;

		DescriptorSetHandle getCurrentDescriptorSet() override;

	protected:
		void markDirty(uint32_t binding);

		void createUniformBuffer(MaterialDataFrame& frameInfo, uint32_t binding, uint32_t size);

	protected:
		Ref<VulkanGraphicsPipeline> m_graphicsPipeline;

		// the uniform buffer data hold in cpu side
		std::unordered_map<uint32_t, MaterialCPUData> m_bindingData;

		// TODO: move to texture?
		VkSampler m_sampler;

		// the layout this material currently using
		VulkanDescriptorSetLayoutHandle m_layout;

		std::vector<MaterialDataFrame> m_materialDataFrame;
	};

}
