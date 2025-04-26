#include "VulkanDescriptorSet.h"

#include "VulkanDescriptorSetManager.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"

namespace PaperEngine {
	VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSetLayoutHandle layout, VkDescriptorPool pool, VkDescriptorSet set) :
		m_set(set), m_pool(pool), m_layout(layout)
	{
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		VulkanContext::GetDescriptorSetManager()->free(m_layout, m_pool, m_set);
	}

	void VulkanDescriptorSet::bindBuffer(uint32_t binding, DescriptorType type, BufferHandle buffer, size_t offset, size_t range)
	{
		PE_CORE_ASSERT(type == DescriptorType::StorageBuffer || type == DescriptorType::UniformBuffer, "Invaild descriptor type.");
		VkDescriptorBufferInfo bufferInfo = {
			.buffer = std::static_pointer_cast<VulkanBuffer>(buffer)->get_handle(),
			.offset = offset,
			.range = range
		};
		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_set,
			.dstBinding = binding,
			.descriptorCount = 1,
			.descriptorType = VulkanDescriptorSetLayout::ConvertType(type),
			.pBufferInfo = &bufferInfo
		};
		vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &write, 0, nullptr);
		m_buffers[binding] = { buffer };
	}

	void VulkanDescriptorSet::bindTextures(uint32_t binding, uint32_t textureCount, TextureHandle* textures)
	{
		std::vector<VkDescriptorImageInfo> imageInfos(textureCount);

		auto& textureList = m_textures[binding];
		textureList.reserve(textureCount);
		for (uint32_t i = 0; i < textureCount; i++) {
			auto& imageInfo = imageInfos[i];
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = std::static_pointer_cast<VulkanTexture>(textures[i])->get_image_view();
			imageInfo.sampler = VK_NULL_HANDLE; // TODO: texture handle need a sampler handle

			textureList.push_back(textures[i]);
		}
		VkWriteDescriptorSet write = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = m_set,
			.dstBinding = binding,
			.descriptorCount = textureCount,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = imageInfos.data()
		};
		vkUpdateDescriptorSets(VulkanContext::GetDevice(), 1, &write, 0, nullptr);

	}

}
