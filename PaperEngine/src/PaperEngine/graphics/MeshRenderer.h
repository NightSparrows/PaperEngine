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

	class MeshRenderer : public IRenderer {
	public:
		MeshRenderer();
		~MeshRenderer();

		// 不對 應該改成process mesh entity之類的
		// 因為需要先使用scene renderer做 culling，不用畫的不會被process
		// 由於是整個scene作process，所以mesh renderer保留process mesh entity的function
		// 然後在自己做static batching之類的
		// 另外skinned mesh (有動畫) 跟這個分開好了，比較不複雜
		void renderScene(std::span<Ref<Scene>> scenes, const GlobalSceneData& globalData) override;
	
		void onBackBufferResized();

	private:

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

		std::unordered_map<Ref<GraphicsPipeline>, ShaderData> m_renderData;

		nvrhi::CommandListHandle m_cmd;
		
		BindingLayoutHandle m_instanceBufBindingLayout;
		nvrhi::BindingSetHandle m_instanceBufferSet;
		// 紀錄instance buffer的transformation
		nvrhi::BufferHandle m_instanceBuffer;
		void* m_instanceBufferCpuPtr{ nullptr };
	};

}
