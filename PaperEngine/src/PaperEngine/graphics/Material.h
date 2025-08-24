#pragma once

#include <unordered_map>

#include <glm/glm.hpp>
#include <variant>

#include <PaperEngine/core/Base.h>
#include <PaperEngine/graphics/GraphicsPipeline.h>

namespace PaperEngine {

	class Material {
	public:

		enum class VariableType {
			Texture,
			Float,
			Vec2,
			Vec3,
			Vec4,
			Mat4,
		};

		struct Variable {
			uint32_t slot; // 參數槽
			VariableType type;

			/// <summary>
			/// 如果是float等類型，這個是在其VariableBuffer中的偏移量
			/// </summary>
			uint32_t offset;
			std::variant<
				nvrhi::TextureHandle, // 如果是Texture
				float,               // 如果是Float
				glm::vec2,          // 如果是Vec2
				glm::vec3,          // 如果是Vec3
				glm::vec4,           // 如果是Vec4
				glm::mat4> value;
		};

	public:
		Material(Ref<GraphicsPipeline> graphicsPipeline, const std::unordered_map<std::string, Variable>& parameterSlots);

		void setFloat(const std::string& name, float value);

		void setTexture(const std::string& name, nvrhi::TextureHandle texture);

		nvrhi::IBindingSet* getBindingSet();

		Ref<GraphicsPipeline> getGraphicsPipeline() { return m_graphicsPipeline; }

	private:
		Ref<GraphicsPipeline> m_graphicsPipeline; // 用於渲染的圖形管線

		/// <summary>
		/// 名稱到參數槽的映射，並紀錄當前的值
		/// 如Texture
		/// </summary>
		std::unordered_map<std::string, Variable> m_parameterSlots;

		nvrhi::BindingSetDesc m_bindingSetDesc; // 綁定佈局描述，用於定義Shader資源的綁定方式

		/// <summary>
		/// 使用graphics pipeline的bindingLayout
		/// 使用currentSwapchainIndex
		/// </summary>
		nvrhi::BindingSetHandle m_bindingSet;

		nvrhi::CommandListHandle m_cmd;

		/// <summary>
		/// 放uniforms的buffer
		/// </summary>
		nvrhi::BufferHandle m_variableBuffer;
		
		std::vector<uint8_t> m_cpuVariableBuffer;
		bool m_variableBufferModified = true;
	};

}
