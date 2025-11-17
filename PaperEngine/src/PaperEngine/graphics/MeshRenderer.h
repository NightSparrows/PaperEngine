#pragma once

#include <nvrhi/nvrhi.h>

#include <span>
#include <vector>
#include <unordered_map>

#include <PaperEngine/graphics/GraphicsPipeline.h>
#include <PaperEngine/graphics/Material.h>
#include <PaperEngine/graphics/Mesh.h>

#include <PaperEngine/scene/Entity.h>
#include <PaperEngine/graphics/Camera.h>
#include <PaperEngine/utils/Transform.h>

#include <PaperEngine/graphics/IRenderer.h>

#include "BindingLayout.h"

namespace PaperEngine {

	/// <summary>
	/// 渲染靜態Mesh的Renderer
	/// </summary>
	class MeshRenderer : public IRenderer {
	public:

		struct InstanceData {
			glm::mat4 trans;
		};

		struct SubMeshData {
			std::vector<InstanceData> instanceData;		// instance的transformation
		};

		struct MeshData {
			std::unordered_map<uint32_t, SubMeshData> subMeshList;
		};

		struct MaterialData {
			std::unordered_map<Ref<Mesh>, MeshData> meshList;
		};

		struct ShaderData {
			std::unordered_map<Ref<Material>, MaterialData> materialList;
		};

	public:
		MeshRenderer();
		~MeshRenderer();

		void addEntity(
			Ref<Material> material,
			Ref<Mesh> mesh,
			uint32_t subMeshIndex,
			const Transform& transform);

		void processScene(Ref<Scene> scene) override;

		// 不對 應該改成process mesh entity之類的
		// 因為需要先使用scene renderer做 culling，不用畫的不會被process
		// 由於是整個scene作process，所以mesh renderer保留process mesh entity的function
		// 然後在自己做static batching之類的
		// 另外skinned mesh (有動畫) 跟這個分開好了，比較不複雜
		void renderScene(const GlobalSceneData& globalData) override;
	
		void onViewportResized(uint32_t width, uint32_t height) override;


		inline uint32_t getTotalInstanceCount() const { return m_totalInstanceCount; }

		inline uint32_t getTotalDrawCallCount() const { return m_totalDrawCallCount; }

	private:

		std::mutex m_add_entity_mutex;
		std::unordered_map<Ref<GraphicsPipeline>, ShaderData> m_renderData;

		// 紀錄renderer 的renderer情況
		uint32_t m_tempInstanceCount{ 0 };
		uint32_t m_totalInstanceCount{ 0 };

		uint32_t m_tempDrawCallCount{ 0 };
		uint32_t m_totalDrawCallCount{ 0 };

		nvrhi::CommandListHandle m_cmd;

		BindingLayoutHandle m_instanceBufBindingLayout;
		nvrhi::BindingSetHandle m_instanceBufferSet;
		// 紀錄instance buffer的transformation
		nvrhi::BufferHandle m_instanceBuffer;
		void* m_instanceBufferCpuPtr{ nullptr };
	};

}
