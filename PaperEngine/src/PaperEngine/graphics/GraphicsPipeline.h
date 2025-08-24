#pragma once

#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	/// <summary>
	/// 放Shader的東西
	/// 只有一個buffer放uniforms (預設放在constant buffer 0)
	/// </summary>
	class GraphicsPipeline {
	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="desc"></param>
		/// <param name="bindingLayout">
		/// 你要自己把Layout放進desc，
		/// 這個單純是你需要設定shader的值之類的（不是全域）
		/// </param>
		GraphicsPipeline(nvrhi::GraphicsPipelineDesc desc, nvrhi::BindingLayoutHandle bindingLayout, size_t variableBufferSize);

		nvrhi::IGraphicsPipeline* getGraphicsPipeline(nvrhi::IFramebuffer* fb) const;

		nvrhi::IBindingLayout* getBindingLayout() const;

		size_t getVariableBufferSize() const { return m_variableBufferSize; }

	private:

		nvrhi::GraphicsPipelineDesc m_graphicsPipelineDesc; // 基本的圖形管線描述，用於創建圖形管線

		mutable nvrhi::GraphicsPipelineHandle m_graphicsPipeline; // 圖形管線，用於渲染圖形

		nvrhi::BindingLayoutHandle m_bindingLayout; // 綁定佈局，用於綁定Shader資源

		size_t m_variableBufferSize;

	};

}
