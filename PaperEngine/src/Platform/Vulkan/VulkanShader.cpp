
#include <fstream>

#include <PaperEngine/core/Logger.h>

#include "VulkanShader.h"
#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace PaperEngine {
	
	VulkanShader::~VulkanShader()
	{
		if (m_handle != VK_NULL_HANDLE) {
			vkDestroyShaderModule(VulkanContext::GetDevice(), m_handle, nullptr);
			PE_CORE_TRACE("shader destroyed. {}", (void*)m_handle);
		}
	}

	bool VulkanShader::load_from_file(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);

		if (!file.is_open()) {
			PE_CORE_ERROR("Failed to open file: {}", filePath);
			return false;
		}

		size_t fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<char> fileContent(fileSize);
		file.read(fileContent.data(), fileSize);

		VkShaderModuleCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = fileSize,
			.pCode = reinterpret_cast<uint32_t*>(fileContent.data())
		};
		CHECK_VK_RESULT(vkCreateShaderModule(VulkanContext::GetDevice(), &createInfo, nullptr, &m_handle));
		PE_CORE_TRACE("Shader created. {}", (void*)m_handle);
		return true;
	}

	VkShaderStageFlags VulkanShader::ConvertStages(ShaderStages stages)
	{
		VkShaderStageFlags returnStages = 0;
		if (stages & ShaderStage::Vertex) {
			returnStages |= VK_SHADER_STAGE_VERTEX_BIT;
		}
		if (stages & ShaderStage::Fragment) {
			returnStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		return returnStages;
	}

}
