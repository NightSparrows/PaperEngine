#pragma once

#include <PaperEngine/imgui/ImGuiLayer.h>
#include <nvrhi/nvrhi.h>

namespace PaperEngine {

	class VulkanImGuiLayer : public ImGuiLayer {
	public:

		void onAttach() override;
		void onDetach() override;

		/// <summary>
		/// Logic update, not rendering.
		/// </summary>
		/// <param name="deltaTime"></param>
		void onUpdate(float deltaTime) override;

		/// <summary>
		/// 用於可以直接在該frame渲染的東西
		/// 不需寫入swapchainImage
		/// </summary>
		void onPreRender() override;

		/// <summary>
		/// 可以寫入swapchainImage的渲染。
		/// </summary>
		void onFinalRender(nvrhi::IFramebuffer*) override;

		/// <summary>
		/// I/O等事件處理。
		/// </summary>
		/// <param name=""></param>
		void onEvent(Event&) override;

		/// <summary>
		/// ImGui function（optional）
		/// </summary>
		void onImGuiRender() override;

		/// <summary>
		/// Swapchain準備要Resize時
		/// </summary>
		void onBackBufferResizing() override;

		/// <summary>
		/// Swapchain resize完成後
		/// </summary>
		void onBackBufferResized() override;

	private:
		bool updateFontTexture();

		bool reallocateBuffer(nvrhi::BufferHandle& buffer, size_t requiredSize, size_t reallocateSize, const bool indexBuffer);

		bool updateGeometry();

		nvrhi::IGraphicsPipeline* getPSO(nvrhi::IFramebuffer* fb);

		nvrhi::IBindingSet* getBindingSet(nvrhi::ITexture* texture);

	private:

		nvrhi::CommandListHandle m_commandList;

		nvrhi::ShaderHandle m_vertexShader;
		nvrhi::ShaderHandle m_fragmentShader;
		nvrhi::InputLayoutHandle m_shaderAttribLayout;

		nvrhi::TextureHandle m_fontTexture;
		nvrhi::SamplerHandle m_fontSampler;

		nvrhi::BufferHandle vertexBuffer;
		nvrhi::BufferHandle indexBuffer;

		nvrhi::BindingLayoutHandle m_bindingLayout;
		nvrhi::GraphicsPipelineDesc m_basePSODesc;

		nvrhi::GraphicsPipelineHandle pso;
		std::unordered_map<nvrhi::ITexture*, nvrhi::BindingSetHandle> bindingCache;

		std::vector<ImDrawVert> vtxBuffer;
		std::vector<ImDrawIdx> idxBuffer;

	};

}
