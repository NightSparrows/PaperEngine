#pragma once

#include <vulkan/vulkan.h>

#include <PaperEngine/renderer/Shader.h>

namespace PaperEngine {

	class VulkanShader : public Shader {
	public:
		~VulkanShader();

		bool load_from_file(const std::string& filePath) override;

		VkShaderModule get_handle() const { return m_handle; }

		static VkShaderStageFlags ConvertStages(ShaderStages stages);

	protected:

		VkShaderModule m_handle{ VK_NULL_HANDLE };

	};

}
