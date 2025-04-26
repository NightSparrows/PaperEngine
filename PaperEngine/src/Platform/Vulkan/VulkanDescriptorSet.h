#pragma once

#include <unordered_map>

#include "VulkanDescriptorSetLayout.h"
#include <PaperEngine/renderer/DescriptorSet.h>

namespace PaperEngine {

	class VulkanDescriptorSet : public DescriptorSet {
	public:
		VulkanDescriptorSet(VulkanDescriptorSetLayoutHandle layout, VkDescriptorPool pool, VkDescriptorSet set);
		~VulkanDescriptorSet();

		void bindBuffer(uint32_t binding, DescriptorType type, BufferHandle buffer, size_t offset, size_t range) override;

		void bindTextures(uint32_t binding, uint32_t textureCount, TextureHandle* textures) override;

		VkDescriptorSet get_handle() const { return m_set; }

	protected:
		VulkanDescriptorSetLayoutHandle m_layout;
		VkDescriptorPool m_pool;
		VkDescriptorSet m_set;

		std::unordered_map<uint32_t, std::vector<BufferHandle>> m_buffers;
		std::unordered_map<uint32_t, std::vector<TextureHandle>> m_textures;


	};

}
