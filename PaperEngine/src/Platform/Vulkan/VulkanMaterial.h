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
		struct MaterialBinding {
			Ref<VulkanBuffer> buffer;
			Ref<VulkanTexture> texture;
		};
		struct MaterialDataFrame {
			bool isUpdated{ false };			// whether this frame is update or not
			std::unordered_map<uint32_t, MaterialBinding> bindings;
			Ref<VulkanDescriptorSet> set;
		};

		VulkanMaterial(const MaterialSpec& spec);
		~VulkanMaterial();

		void updateData(uint32_t binding, const void* data, size_t size, size_t offset) override;

		void updateTexture(uint32_t binding, TextureHandle texture) override;

		Ref<VulkanDescriptorSet> getCurrentDescriptorSet();

	protected:
		void markDirty();

	protected:
		Ref<VulkanGraphicsPipeline> m_graphicsPipeline;

		// the uniform buffer data hold in cpu side
		std::unordered_map<uint32_t, std::vector<char>> m_buffers;
		std::unordered_map<uint32_t, VulkanTextureHandle> m_textures;

		VkSampler m_sampler;

		// the layout this material currently using
		VulkanDescriptorSetLayoutHandle m_layout;

		std::vector<MaterialDataFrame> m_materialDataFrame;
	};

}
