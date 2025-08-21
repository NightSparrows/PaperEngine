#pragma once

#include <nvrhi/nvrhi.h>

namespace PaperEngine {


	/// <summary>
	/// 用於Material的Shader。
	/// create graphics pipeline 需要強制使用framebuffer真的很白癡的設計= =
	/// 使用dummy framebuffer來創建graphics pipeline。
	/// </summary>
	class MaterialShader {
	public:
		~MaterialShader() = default;
		/// <summary>
		/// 獲取綁定佈局。
		/// </summary>
		nvrhi::BindingLayoutHandle getBindingLayout() const { return m_bindingLayout; }
		/// <summary>
		/// 獲取圖形管線。
		/// </summary>
		nvrhi::GraphicsPipelineHandle getGraphicsPipeline() const { return m_graphicsPipeline; }

	private:
		nvrhi::BindingLayoutHandle m_bindingLayout;
		nvrhi::GraphicsPipelineHandle m_graphicsPipeline;

	};

}
