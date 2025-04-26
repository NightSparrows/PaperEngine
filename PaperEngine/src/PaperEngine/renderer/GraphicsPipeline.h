#pragma once

#include <string>
#include <vector>

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Shader.h>
#include <PaperEngine/renderer/DescriptorSetLayout.h>

namespace PaperEngine {

	struct GraphicsPipelineSpecification
	{
		struct VariableInfo {
			uint32_t binding;
			uint32_t count;
			DescriptorType type;
			ShaderStages stages;
			uint32_t bufferSize;
		};

		typedef enum class Topology {
			TriangleList
		}Topology;
		
		// TODO: shaders (vertex, fragment)
		ShaderHandle VS;
		ShaderHandle FS;
		
		GraphicsPipelineSpecification& setVertexShader(ShaderHandle shader) {
			VS = shader;
			return *this;
		}

		GraphicsPipelineSpecification& setFragmentShader(ShaderHandle shader) {
			FS = shader;
			return *this;
		}

		GraphicsPipelineSpecification& setCacheFilePath(const std::string& filePath) {
			cacheFilePath = filePath;
			return *this;
		}

		/// <summary>
		/// Add the graphics pipeline material variables
		/// set is set to 1
		/// binding is binding
		/// </summary>
		/// <param name="type"></param>
		/// <param name="binding"></param>
		/// <param name="stages"></param>
		/// <param name="count"></param>
		/// <returns></returns>
		GraphicsPipelineSpecification& addMaterialTexture(uint32_t binding, ShaderStages stages, uint32_t count = 1) {
			auto& varInfo = materialVariables.emplace_back();
			varInfo.type = DescriptorType::CombinedImageSampler;
			varInfo.binding = binding;
			varInfo.count = count;
			varInfo.stages = stages;

			return *this;
		}
		GraphicsPipelineSpecification& addMaterialUniformBuffer(uint32_t binding, ShaderStages stages, uint32_t size, uint32_t count = 1) {
			auto& varInfo = materialVariables.emplace_back();
			varInfo.type = DescriptorType::UniformBuffer;
			varInfo.binding = binding;
			varInfo.bufferSize = size;
			varInfo.count = count;
			varInfo.stages = stages;

			return *this;
		}

		Topology topology{ Topology::TriangleList };
		bool primitiveRestartEnable{ false };

		typedef enum class CullMode {
			None,
			Back
		}CullMode;

		CullMode cullMode{ CullMode::Back };

		GraphicsPipelineSpecification& setCullMode(CullMode mode) {
			cullMode = mode;
			return *this;
		}

		// depth testing, depth writing
		bool depthTest{ true };
		bool depthWrite{ true };

		// cache file
		// use for faster graphics pipeline creation
		// setting the file will create the cache file to it
		// optional
		std::string cacheFilePath;

		// the variables per materials
		std::vector<VariableInfo> materialVariables;
	};

	/// <summary>
	/// Use for general mesh shading
	/// </summary>
	class GraphicsPipeline {
	public:
		virtual ~GraphicsPipeline() = default;

		PE_API static Ref<GraphicsPipeline> Create(const GraphicsPipelineSpecification& spec);

	protected:
		GraphicsPipeline() = default;

	};

	typedef Ref< GraphicsPipeline> GraphicsPipelineHandle;
}
