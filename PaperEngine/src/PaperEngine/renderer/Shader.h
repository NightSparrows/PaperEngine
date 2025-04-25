#pragma once

#include <string>

#include <PaperEngine/core/Base.h>

namespace PaperEngine {

	typedef enum ShaderStage : uint32_t {
		Vertex = 1,
		Fragment = 2
	}ShaderStage;

	typedef uint32_t ShaderStages;

	class Shader {
	public:
		virtual ~Shader() = default;

		virtual bool load_from_file(const std::string& filePath) = 0;

		PE_API static Ref<Shader> Create();

	protected:
		Shader() = default;
	};

	typedef Ref<Shader> ShaderHandle;
}
