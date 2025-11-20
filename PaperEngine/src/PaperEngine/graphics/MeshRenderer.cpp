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
		nvrhi::BufferDesc instanceBufferDesc;
		instanceBufferDesc
			.setByteSize(sizeof(InstanceData) * 100000)
			.setIsConstantBuffer(true)
			.setKeepInitialState(true)
			.setStructStride(sizeof(InstanceData))
			.setCpuAccess(nvrhi::CpuAccessMode::Write);
		m_instanceBuffer = CreateRef<GPUBuffer>(ResourceUsage::FrameStreaming, instanceBufferDesc);

		nvrhi::BindingLayoutDesc instanceBufLayoutDesc;
		instanceBufLayoutDesc
			.setRegisterSpace(1)			// set = 1
			.setRegisterSpaceIsDescriptorSet(true)
			.setVisibility(nvrhi::ShaderType::Vertex)
			.addItem(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(0));
		m_instanceBufBindingLayout = 
			Application::GetResourceManager()->create<BindingLayout>("MeshRenderer_instanceBufLayout",
				Application::GetNVRHIDevice()->createBindingLayout(instanceBufLayoutDesc));

		const uint32_t max_frame_count = Application::Get()->getGraphicsContext()->getMaxFrameInFlight();
		std::vector<nvrhi::BindingSetDesc> instanceBufSetDescs(max_frame_count);
		for (uint32_t i = 0; i < max_frame_count; i++)
		{
			nvrhi::BindingSetDesc& instanceBufSetDesc = instanceBufSetDescs[i];
			instanceBufSetDesc.addItem(nvrhi::BindingSetItem::StructuredBuffer_SRV(0, m_instanceBuffer->getStorages()[i].handle));
		}
		m_instanceBufferSet = std::make_shared<BindingSet>(ResourceUsage::FrameStreaming, m_instanceBufBindingLayout, instanceBufSetDescs);

	}

	MeshRenderer::~MeshRenderer()
	{
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

	void MeshRenderer::processScene(Ref<Scene> scene, const Frustum& camera_frustum)
	{
		const auto scene_group = scene->getRegistry().group<MeshComponent>(entt::get<TransformComponent, MeshRendererComponent>);

		auto group_start = scene_group.begin();
		auto group_end = scene_group.end();

		size_t thread_count = Application::GetThreadPool()->get_thread_count();
		size_t chunk_size = (scene_group.size() + thread_count) / thread_count;

		std::vector<std::future<void>> process_futures(thread_count);

		for (size_t i = 0; i < thread_count; i++)
		{
			PE_PROFILE_SCOPE("static mesh scene dispatch.");

			auto start_it = group_start;
			std::advance(group_start, chunk_size);
			process_futures[i] = Application::GetThreadPool()->submit_task([&, group_start, group_end, thread_count, start_it, i]()
				{
					PE_PROFILE_SCOPE("Worker thread process mesh renderers");
					auto end_it = (i == thread_count - 1) ? group_end : group_start;
					for (auto it = start_it; it != end_it; ++it)
					{
						auto entity = *it;
						const auto& meshCom = scene_group.get<MeshComponent>(entity);
						const auto& meshRendererCom = scene_group.get<MeshRendererComponent>(entity);
						const auto mesh = meshCom.mesh;
						const auto& transform = scene_group.get<TransformComponent>(entity).transform;

						if (!meshRendererCom.visible)
							continue;

						// Frustum culling for meshes
						if (!camera_frustum.isIntersect(meshCom.worldAABB))
							continue;
						if (!meshRendererCom.renderStatic)		// 不是作為static mesh來render的
							continue;
						// meshRenderer的materials跟subMesh是一對一的
						PE_CORE_ASSERT(mesh->getSubMeshes().size() == meshRendererCom.materials.size(), "Wired mesh renderer materials doesn't match mesh submeshes");
						// m_forwardPlusDepthRenderer.addEntity(mesh, transform);
						for (uint32_t subMeshIndex = 0; subMeshIndex < mesh->getSubMeshes().size(); subMeshIndex++) {
							auto material = meshRendererCom.materials[subMeshIndex];
							if (!material || !material->getBindingSet())
								continue;			// TODO: 改成null material之類的可以顯示
							this->addEntity(
								material,
								mesh,
								subMeshIndex,
								transform);
						}
					}
				});
		}

		// 等待process mesh完成
		for (auto& future : process_futures)
		{
			future.get();
		}
	}

	void MeshRenderer::renderScene(nvrhi::ICommandList* cmd, const GlobalSceneData& globalData)
	{
		PE_PROFILE_FUNCTION();


		// rendering

		// 確保texture state正確
		auto color_texture = globalData.fb->getDesc().colorAttachments[0].texture;
		auto depth_texture = globalData.fb->getDesc().depthAttachment.texture;
		cmd->setTextureState(color_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::RenderTarget);
		cmd->setTextureState(depth_texture, nvrhi::AllSubresources, nvrhi::ResourceStates::DepthWrite);

		cmd->commitBarriers();

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
		graphicsState.bindings[1] = m_instanceBufferSet->getHandle();

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
							static_cast<uint8_t*>(m_instanceBuffer->getMapPtr()) + instanceOffset * sizeof(InstanceData),
							subMeshData.instanceData.data(),
							transMatSize);


						mesh->bindSubMesh(drawArgs, subMesh);
						drawArgs.setStartInstanceLocation(instanceOffset);
						drawArgs.setInstanceCount(static_cast<uint32_t>(instanceCount));
						cmd->setGraphicsState(graphicsState);
						cmd->drawIndexed(drawArgs);
						m_tempDrawCallCount++;
						instanceOffset += instanceCount;
					}
				}
			}
		}

	}

	void MeshRenderer::endFrame()
	{
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
