#include "VulkanMaterial.h"

#include "VulkanContext.h"
#include "VulkanUtils.h"
#include "VulkanDescriptorSet.h"
#include "VulkanTexture.h"

namespace PaperEngine {

	VulkanMaterial::VulkanMaterial(const MaterialSpec& spec)
	{
		m_graphicsPipeline = std::static_pointer_cast<VulkanGraphicsPipeline>(spec.graphicsPipeline);
		m_layout = m_graphicsPipeline->get_material_layout();

		m_materialDataFrame.reserve(VulkanContext::GetImageCount());
		for (uint32_t i = 0; i < VulkanContext::GetImageCount(); i++) {
			auto& frameInfo = m_materialDataFrame.emplace_back();
			frameInfo.set = VulkanContext::GetDescriptorSetManager()->allocate(m_layout);
			
			for (const auto& [binding, materialBindingInfo] : m_graphicsPipeline->get_material_binding_infos()) {
				if (materialBindingInfo.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					BufferSpecification uniformBufferSpec = {
						.size = materialBindingInfo.size,
						.isUniformBuffer = true
					};
					frameInfo.bindings[binding].buffer = CreateRef<VulkanBuffer>(uniformBufferSpec);
					auto& bufferDataBuffer = m_buffers[binding];
					bufferDataBuffer.resize(materialBindingInfo.size);
				}
			}
		}

		// create sampler TODO: move it to texture object
		VkSamplerCreateInfo samplerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,  // Not used since no mips
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_FALSE,
			.maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		CHECK_VK_RESULT(vkCreateSampler(VulkanContext::GetDevice(), &samplerCreateInfo, nullptr, &m_sampler));
	}

	VulkanMaterial::~VulkanMaterial()
	{
		vkDestroySampler(VulkanContext::GetDevice(), m_sampler, nullptr);
	}

	void VulkanMaterial::updateData(uint32_t binding, const void* data, size_t size, size_t offset)
	{
		memcpy_s(m_buffers[binding].data() + offset, m_buffers[binding].size() - offset, data, size);
		markDirty();
	}

	void VulkanMaterial::updateTexture(uint32_t slot, TextureHandle texture)
	{
		m_textures[slot] = std::static_pointer_cast<VulkanTexture>(texture);
		markDirty();
	}

	GraphicsPipelineHandle VulkanMaterial::get_graphics_pipeline() const
	{
		return m_graphicsPipeline;
	}

	DescriptorSetHandle VulkanMaterial::getCurrentDescriptorSet()
	{
		auto& frameInfo = m_materialDataFrame[VulkanContext::GetCurrentImageIndex()];

		if (!frameInfo.isUpdated) {
			std::vector<VkWriteDescriptorSet> writes;
			std::vector<VkDescriptorBufferInfo> bufferInfo;
			bufferInfo.resize(m_buffers.size());
			uint32_t bufferInfoIndex = 0;
			// upload uniform buffer
			VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();
			cmd->open();
			for (const auto& [binding, bufferData] : m_buffers) {
				cmd->writeBuffer(frameInfo.bindings[binding].buffer, m_buffers[binding].data(), m_buffers[binding].size());

				// update uniform buffer to descriptor set
				bufferInfo[bufferInfoIndex] = {
					.buffer = frameInfo.bindings[binding].buffer->get_handle(),
					.offset = 0,
					.range = frameInfo.bindings[binding].buffer->get_size()
				};
				writes.emplace_back(VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frameInfo.set->get_handle(),
					.dstBinding = binding,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.pBufferInfo = &bufferInfo[bufferInfoIndex]
					});
				bufferInfoIndex++;
			}
			cmd->close();
			VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);



			// textures
			std::vector<VkDescriptorImageInfo> imageInfo;
			imageInfo.resize(m_textures.size());
			uint32_t imageInfoIndex = 0;

			for (const auto& [index, texture] : m_textures) {
				imageInfo[imageInfoIndex] = {
					.sampler = m_sampler,
					.imageView = texture->get_image_view(),
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				};
				writes.emplace_back(VkWriteDescriptorSet{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = frameInfo.set->get_handle(),
					.dstBinding = index,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &imageInfo[imageInfoIndex]
					});
				imageInfoIndex++;
			}

			
			vkUpdateDescriptorSets(VulkanContext::GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

			frameInfo.isUpdated = true;
		}

		return frameInfo.set;
	}

	void VulkanMaterial::markDirty()
	{
		for (auto& frameInfo : m_materialDataFrame) {
			frameInfo.isUpdated = false;
		}
	}

}
