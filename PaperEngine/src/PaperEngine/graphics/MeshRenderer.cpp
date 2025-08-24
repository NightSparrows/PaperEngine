#include "MeshRenderer.h"

#include <PaperEngine/core/Application.h>

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/MeshComponent.h>
#include <PaperEngine/components/MeshRendererComponent.h>

namespace PaperEngine {
	
	MeshRenderer::MeshRenderer()
	{
		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		nvrhi::BufferDesc instanceBufferDesc;
		instanceBufferDesc
			.setByteSize(sizeof(glm::mat4) * 10000)
			.setIsConstantBuffer(true);
		m_instanceBuffer = Application::GetNVRHIDevice()->createBuffer(instanceBufferDesc);

		nvrhi::BindingLayoutDesc instanceBufLayoutDesc;
		instanceBufLayoutDesc.setVisibility(nvrhi::ShaderType::Vertex);
		instanceBufLayoutDesc.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0));
		m_instanceBufBindingLayout = Application::GetNVRHIDevice()->createBindingLayout(instanceBufLayoutDesc);

		nvrhi::BindingSetDesc instanceBufSetDesc;
		instanceBufSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_instanceBuffer));
		m_instanceBufferSet = Application::GetNVRHIDevice()->createBindingSet(instanceBufSetDesc, m_instanceBufBindingLayout);

	}

	void MeshRenderer::renderScene(std::span<Ref<Scene>> scenes, const GlobalData& globalData)
	{
		m_renderData.clear();

		// process mesh
		for (auto scene : scenes) {
			auto sceneView = scene->getRegistry().view<
				TransformComponent,
				MeshComponent,
				MeshRendererComponent>();
			for (auto [entity, transformCom, meshCom, meshRendererCom] : sceneView.each()) {
				const auto& transform = transformCom.transform;
				const auto& mesh = meshCom.mesh;

				// 不處理有動畫的mesh
				if (mesh->getType() == MeshType::Skeletal)
					continue;

				// TODO: camera frustum culling

				PE_CORE_ASSERT(mesh->getSubMeshes().size() == meshRendererCom.materials.size(), "Wired mesh renderer materials doesn't match mesh submeshes");

				// meshRenderer的materials跟subMesh是一對一的
				for (uint32_t subMeshIndex = 0; subMeshIndex < mesh->getSubMeshes().size(); subMeshIndex++) {
					auto material = meshRendererCom.materials[subMeshIndex];

					if (!material)
						continue;			// TODO: 改成null material之類的可以顯示

					auto& materialList = m_renderData[material->getGraphicsPipeline()].materialList;
					auto& meshList = materialList[material].meshList;
					auto& subMeshList = meshList[mesh].subMeshList;
					auto& subMeshData = subMeshList[subMeshIndex];

					subMeshData.instanceData.emplace_back(transform);
				}
			}
		}

		m_cmd->open();


		// rendering
		nvrhi::GraphicsState graphicsState;
		nvrhi::DrawArguments drawArgs;

		/// 0: globalSet
		/// 1: instance buffer
		/// 2: material
		graphicsState.bindings.resize(3);
		graphicsState.bindings[0] = globalData.globalSet;
		graphicsState.bindings[1] = m_instanceBufferSet;

		// Render
		size_t instanceOffset = 0;
		for (auto& [graphicsPipeline, shaderData] : m_renderData) {
			graphicsState.setPipeline(graphicsPipeline->getGraphicsPipeline(globalData.fb));

			for (auto& [material, materialData] : shaderData.materialList) {
				graphicsState.bindings[2] = material->getBindingSet();
				for (auto& [mesh, meshData] : materialData.meshList) {
					mesh->bindMesh(graphicsState);
					m_cmd->setGraphicsState(graphicsState);
					for (auto& [subMesh, subMeshData] : meshData.subMeshList) {

						// instance buffer uploading
						size_t instanceCount = subMeshData.instanceData.size();
						size_t transMatSize = instanceCount * sizeof(InstanceData);

						m_cmd->writeBuffer(
							m_instanceBuffer,
							subMeshData.instanceData.data(),
							transMatSize,
							instanceCount * sizeof(InstanceData));


						mesh->bindSubMesh(drawArgs, subMesh);
						drawArgs.setStartInstanceLocation(instanceOffset);
						drawArgs.setInstanceCount(static_cast<uint32_t>(instanceCount));
						m_cmd->drawIndexed(drawArgs);

						instanceOffset += instanceCount;
					}
				}
			}
		}
		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);
	}

}
