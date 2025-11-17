#include "MeshRenderer.h"

#include <PaperEngine/core/Application.h>

#include <PaperEngine/components/TransformComponent.h>
#include <PaperEngine/components/MeshComponent.h>
#include <PaperEngine/components/MeshRendererComponent.h>

#include <PaperEngine/utils/BoundingVolume.h>

#include <PaperEngine/debug/Instrumentor.h>

namespace PaperEngine {
	
	MeshRenderer::MeshRenderer()
	{
		m_cmd = Application::GetNVRHIDevice()->createCommandList();

		nvrhi::BufferDesc instanceBufferDesc;
		instanceBufferDesc
			.setByteSize(sizeof(InstanceData) * 100000)
			.setIsConstantBuffer(true)
			.setKeepInitialState(true)
			.setStructStride(sizeof(InstanceData))
			.setCpuAccess(nvrhi::CpuAccessMode::Write);
		m_instanceBuffer = Application::GetNVRHIDevice()->createBuffer(instanceBufferDesc);
		m_instanceBufferCpuPtr = Application::GetNVRHIDevice()->mapBuffer(m_instanceBuffer, nvrhi::CpuAccessMode::Write);

		nvrhi::BindingLayoutDesc instanceBufLayoutDesc;
		instanceBufLayoutDesc
			.setRegisterSpace(1)			// set = 1
			.setRegisterSpaceIsDescriptorSet(true)
			.setVisibility(nvrhi::ShaderType::Vertex)
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0));
		m_instanceBufBindingLayout = 
			Application::GetResourceManager()->create<BindingLayout>("MeshRenderer_instanceBufLayout",
				Application::GetNVRHIDevice()->createBindingLayout(instanceBufLayoutDesc));

		nvrhi::BindingSetDesc instanceBufSetDesc;
		instanceBufSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_instanceBuffer));
		m_instanceBufferSet = Application::GetNVRHIDevice()->createBindingSet(instanceBufSetDesc, m_instanceBufBindingLayout->handle);

	}

	MeshRenderer::~MeshRenderer()
	{
		Application::GetNVRHIDevice()->unmapBuffer(m_instanceBuffer);
		m_instanceBufferCpuPtr = nullptr;
	}

	void MeshRenderer::addEntity(Ref<Material> material, Ref<Mesh> mesh, uint32_t subMeshIndex, const Transform& transform)
	{
		std::lock_guard<std::mutex> lock(m_add_entity_mutex);
		auto& materialList = m_renderData[material->getGraphicsPipeline()].materialList;
		auto& meshList = materialList[material].meshList;
		auto& subMeshList = meshList[mesh].subMeshList;
		auto& subMeshData = subMeshList[subMeshIndex];

		subMeshData.instanceData.emplace_back(transform);
		m_tempInstanceCount++;
	}

	void MeshRenderer::processScene(Ref<Scene> scene)
	{
		const auto scene_group = scene->getRegistry().group<MeshComponent>(entt::get<TransformComponent, MeshRendererComponent>);


	}

	void MeshRenderer::renderScene(const GlobalSceneData& globalData)
	{
		PE_PROFILE_FUNCTION();
		// rendering
		m_cmd->open();


		nvrhi::GraphicsState graphicsState;
		graphicsState.setFramebuffer(globalData.fb);
		nvrhi::DrawArguments drawArgs;
		graphicsState.viewport.addViewportAndScissorRect(
			nvrhi::Viewport(
				0,
				globalData.camera->getWidth(),
				0,
				globalData.camera->getHeight(),
				0,
				1));

		/// 0: globalSet
		/// 1: instance buffer
		/// 2: material
		graphicsState.bindings.resize(3);
		graphicsState.bindings[0] = globalData.globalSet;
		graphicsState.bindings[1] = m_instanceBufferSet;

		// Render
		size_t instanceOffset = 0;
		for (auto& [graphicsPipeline, shaderData] : m_renderData) {
			graphicsPipeline->bind(graphicsState, globalData.fb);

			for (auto& [material, materialData] : shaderData.materialList) {
				graphicsState.bindings[2] = material->getBindingSet();
				for (auto& [mesh, meshData] : materialData.meshList) {
					mesh->bindMesh(graphicsState);
					for (auto& [subMesh, subMeshData] : meshData.subMeshList) {

						// instance buffer uploading
						size_t instanceCount = subMeshData.instanceData.size();
						size_t transMatSize = instanceCount * sizeof(InstanceData);

						memcpy(
							static_cast<uint8_t*>(m_instanceBufferCpuPtr) + instanceOffset * sizeof(InstanceData),
							subMeshData.instanceData.data(),
							transMatSize);


						mesh->bindSubMesh(drawArgs, subMesh);
						drawArgs.setStartInstanceLocation(instanceOffset);
						drawArgs.setInstanceCount(static_cast<uint32_t>(instanceCount));
						m_cmd->setGraphicsState(graphicsState);
						m_cmd->drawIndexed(drawArgs);
						m_tempDrawCallCount++;
						instanceOffset += instanceCount;
					}
				}
			}
		}
		m_cmd->close();
		Application::GetNVRHIDevice()->executeCommandList(m_cmd);
		Application::GetNVRHIDevice()->waitForIdle();

		m_renderData.clear();
		m_totalInstanceCount = m_tempInstanceCount;
		m_tempInstanceCount = 0;
		m_totalDrawCallCount = m_tempDrawCallCount;
		m_tempDrawCallCount = 0;
	}

	void MeshRenderer::onViewportResized(uint32_t width, uint32_t height)
	{

	}
}
