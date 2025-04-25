#pragma once

#include <unordered_map>

#include <vulkan/vulkan.h>

#include <PaperEngine/renderer/GraphicsPipeline.h>
#include "VulkanDescriptorSetLayout.h"

namespace PaperEngine {

	class VulkanGraphicsPipeline : public GraphicsPipeline {
	public:
		struct MaterialBindingInfo {
			VkDescriptorType type;
			// if the type is buffer what are the buffer
			uint32_t size;
		};

		VulkanGraphicsPipeline(const GraphicsPipelineSpecification& spec);
		~VulkanGraphicsPipeline();

		VkPipeline get_handle() const { return m_handle; }

		VkPipelineLayout get_layout() const { return m_layout; }

		VulkanDescriptorSetLayoutHandle get_material_layout() const { return m_materialLayout; }

		const std::unordered_map<uint32_t, MaterialBindingInfo>& get_material_binding_infos() { return m_materialBindings; }
	protected:
		VkPipeline m_handle{ VK_NULL_HANDLE };

		std::string m_cacheFilePath;
		VkPipelineCache m_pipelineCache{ VK_NULL_HANDLE };

		VkPipelineLayout m_layout{ VK_NULL_HANDLE };

		VulkanDescriptorSetLayoutHandle m_materialLayout;

		// if having the variables, the binding slot is 0
		std::unordered_map<uint32_t, MaterialBindingInfo> m_materialBindings;

	};

}
