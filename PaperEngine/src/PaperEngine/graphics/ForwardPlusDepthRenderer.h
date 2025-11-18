#pragma once

#include "IRenderer.h"

#include "MeshRenderer.h"

#include "GraphicsPipeline.h"

#include <nvrhi/nvrhi.h>



namespace PaperEngine {

	class ForwardPlusDepthRenderer : public IRenderer {
	public:

		struct InstanceData {
			glm::mat4 trans;
		};

		struct MeshData {
			std::vector<InstanceData> instanceData;		// instance的transformation
		};

	public:
		ForwardPlusDepthRenderer();
		~ForwardPlusDepthRenderer();

		void init();

		/// <summary>
		/// Thread safe
		/// </summary>
		/// <param name="mesh"></param>
		/// <param name="transform"></param>
		void addEntity(
			Ref<Mesh> mesh,
			const Transform& transform);

		void renderScene(nvrhi::ICommandList* cmd, const GlobalSceneData& globalData) override;

		void onViewportResized(uint32_t width, uint32_t height) override;

	private:
		void createFramebuffer();

	private:
		std::unordered_map<Ref<Mesh>, MeshData> m_renderData;

		struct FramebufferInfo {
			nvrhi::TextureHandle depthTexture;
			nvrhi::FramebufferHandle handle;
		};

		FramebufferInfo m_framebuffer;

		nvrhi::CommandListHandle m_cmd;

		// depth only render pass
		Ref<GraphicsPipeline> m_graphicsPipeline;

		BindingLayoutHandle m_instanceBufBindingLayout;
		nvrhi::BindingSetHandle m_instanceBufferSet;

		nvrhi::BufferHandle m_instanceBuffer;
		void* m_instanceBufferCpuPtr = nullptr;

		uint32_t m_width{ 0 }, m_height{ 0 };

		std::mutex m_add_entity_mutex;
	};

}
