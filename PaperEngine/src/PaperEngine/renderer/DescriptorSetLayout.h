#pragma once

#include <cstdint>

#include <PaperEngine/core/Base.h>

#include <PaperEngine/renderer/Shader.h>

namespace PaperEngine {

	typedef enum DescriptorType {
		UniformBuffer,
		StorageBuffer,
		CombinedImageSampler
	}DescriptorType;

	struct DescriptorSetLayoutBinding {
		uint32_t binding;
		DescriptorType type;
		uint32_t count;
		ShaderStages stages;
	};

	struct DescriptorSetLayoutSpec {
		std::vector<DescriptorSetLayoutBinding> bindings;

		DescriptorSetLayoutSpec& addUniformBuffer(uint32_t binding, ShaderStages stages, uint32_t count = 1) {
			bindings.emplace_back(DescriptorSetLayoutBinding{ binding,DescriptorType::UniformBuffer, count, stages });

			return *this;
		}

		DescriptorSetLayoutSpec& addStorageBuffer(uint32_t binding, ShaderStages stages, uint32_t count = 1) {
			bindings.emplace_back(DescriptorSetLayoutBinding{ binding,DescriptorType::StorageBuffer, count, stages });

			return *this;
		}

		DescriptorSetLayoutSpec& addImageSampler(uint32_t binding, ShaderStages stages, uint32_t count = 1) {
			bindings.emplace_back(DescriptorSetLayoutBinding{ binding,DescriptorType::CombinedImageSampler, count, stages });

			return *this;
		}

	};

	class DescriptorSetLayout {
	public:
		virtual ~DescriptorSetLayout() = default;
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

		static Ref<DescriptorSetLayout> Create(const DescriptorSetLayoutSpec& spec);

	protected:
		DescriptorSetLayout() = default;
	};

	typedef Ref<DescriptorSetLayout> DescriptorSetLayoutHandle;
}
