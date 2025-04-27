#include "VulkanMeshRenderer.h"

#include <PaperEngine/component/MeshComponent.h>
#include "VulkanContext.h"
#include <PaperEngine/component/TransformComponent.h>

namespace PaperEngine {

	static VulkanDescriptorSetLayoutHandle s_meshInstanceDescriptorSetLayout;

	void VulkanMeshRenderer::prepareScene(const Scene& scene)
	{
		m_renderCommands.clear();

		VulkanCommandBufferHandle cmd = CreateRef<VulkanCommandBuffer>();
		cmd->open();

		for (const auto& [entity, transformCom, meshCom] : scene.get_registry().view<TransformComponent, MeshComponent>().each()) {
			const auto& transform = transformCom.transform;
			auto& renderCmd = m_renderCommands.emplace_back();
			for (uint32_t i = 0; i < meshCom.mesh->get_sub_meshes().size(); i++) {
				const auto& material = meshCom.materials[i];
				renderCmd.graphicsPipeline = material->get_graphics_pipeline();
				renderCmd.material = material;
				renderCmd.mesh = meshCom.mesh;
				renderCmd.submeshIndex = i;

				if (meshCom.instanceSets.size() != VulkanContext::GetImageCount()) {
					meshCom.instanceSets.resize(VulkanContext::GetImageCount());
				}
				auto set = meshCom.instanceSets[VulkanContext::GetCurrentImageIndex()];
				if (!set) {
					set = VulkanContext::GetDescriptorSetManager()->allocate(s_meshInstanceDescriptorSetLayout);
					meshCom.instanceSets[VulkanContext::GetCurrentImageIndex()] = set;

					// means that we also don't have the buffer yet
					BufferSpecification meshInfoUniormBufferSpec;
					meshInfoUniormBufferSpec
						.setSize(sizeof(InstanceInfoUniformStruct))
						.setIsUniformBuffer(true);
					BufferHandle meshInfoUniformBuffer = Buffer::Create(meshInfoUniormBufferSpec);
					if (meshCom.instanceInfoBuffer.size() != VulkanContext::GetImageCount()) {
						meshCom.instanceInfoBuffer.resize(VulkanContext::GetImageCount());
					}
					meshCom.instanceInfoBuffer[VulkanContext::GetCurrentImageIndex()] = meshInfoUniformBuffer;
					
					// bind the buffers to set
					set->bindBuffer(0, DescriptorType::UniformBuffer, meshInfoUniformBuffer, 0, sizeof(InstanceInfoUniformStruct));
					const auto basicVertexBuffer = meshCom.mesh->get_basic_vertex_buffer();
					set->bindBuffer(1, DescriptorType::StorageBuffer, basicVertexBuffer, 0, basicVertexBuffer->get_size());
					if (meshCom.mesh->get_type() == MeshType::Animated) {
						const auto boneVertexBuffer = meshCom.mesh->get_bone_vertex_buffer();
						PE_CORE_ASSERT(meshCom.boneTransformBuffer.size() == VulkanContext::GetImageCount(), "bone transform buffer is not created while this mesh instance is animted.");
						const auto boneTransformBuffer = meshCom.boneTransformBuffer[VulkanContext::GetCurrentImageIndex()];
						set->bindBuffer(2, DescriptorType::StorageBuffer, boneVertexBuffer, 0, boneVertexBuffer->get_size());
						set->bindBuffer(3, DescriptorType::StorageBuffer, boneTransformBuffer, 0, boneTransformBuffer->get_size());
					}
				}
				InstanceInfoUniformStruct instanceInfo{};
				instanceInfo.transform = transform;
				instanceInfo.meshType = meshCom.mesh->get_type();
				cmd->writeBuffer(meshCom.instanceInfoBuffer[VulkanContext::GetCurrentImageIndex()], &instanceInfo, sizeof(InstanceInfoUniformStruct));

				renderCmd.instanceSet = set;
			}
		}

		cmd->close();
		VulkanContext::GetCommandBufferManager()->executeCommandBuffer(cmd);

		std::sort(m_renderCommands.begin(), m_renderCommands.end(), [](const RenderCommand& a, const RenderCommand& b) {
			return std::tie(a.graphicsPipeline, a.material, b.instanceSet) <
				std::tie(b.graphicsPipeline, b.material, b.instanceSet);
			});
	}

	void VulkanMeshRenderer::render(CommandBufferHandle cmd, DescriptorSetHandle globalSet)
	{
		GraphicsPipelineHandle currentGraphicsPipeline = nullptr;
		MaterialHandle currentMaterial = nullptr;
		DescriptorSetHandle currentInstanceSet = nullptr;
		for (const auto& renderCmd : m_renderCommands) {
			if (currentGraphicsPipeline != renderCmd.graphicsPipeline) {
				currentGraphicsPipeline = renderCmd.graphicsPipeline;
				cmd->bindGraphicsPipeline(currentGraphicsPipeline);
				// TODO: activate scissor
				VkRect2D scissor = {
					.extent = {
						.width = 2560,
						.height = 1440
					}
				};
				VkViewport viewport = {
					.width = 2560,
					.height = 1440,
					.maxDepth = 1.f
				};
				vkCmdSetScissor(std::static_pointer_cast<VulkanCommandBuffer>(cmd)->get_handle(), 0, 1, &scissor);
				vkCmdSetViewport(std::static_pointer_cast<VulkanCommandBuffer>(cmd)->get_handle(), 0, 1, &viewport);
				cmd->bindDescriptorSet(0, globalSet);
			}
			if (currentMaterial != renderCmd.material) {
				currentMaterial = renderCmd.material;
				cmd->bindDescriptorSet(1, currentMaterial->getCurrentDescriptorSet());
			}
			if (currentInstanceSet != renderCmd.instanceSet) {
				currentInstanceSet = renderCmd.instanceSet;
				cmd->bindDescriptorSet(2, currentInstanceSet);
				cmd->bindIndexBuffer(renderCmd.mesh->get_index_buffer());
			}

			const auto& subMeshInfo = renderCmd.mesh->get_sub_meshes()[renderCmd.submeshIndex];
			cmd->drawIndexed(subMeshInfo.count, subMeshInfo.offset);
		}
	}

	DescriptorSetHandle VulkanMeshRenderer::allocateInstanceSet()
	{
		return VulkanContext::GetDescriptorSetManager()->allocate(s_meshInstanceDescriptorSetLayout);
	}

	void VulkanMeshRenderer::Init()
	{

#pragma region Global Descriptor Set Layout Creation
		DescriptorSetLayoutSpec instanceDscLayoutSpec{};

		// for small information
		instanceDscLayoutSpec
			.addUniformBuffer(0, ShaderStage::Vertex)							// for static instance information buffer (transformation)
			.addStorageBuffer(1, ShaderStage::Vertex)							// for vertex basic buffer (per vertex)
			.addStorageBuffer(2, ShaderStage::Vertex)							// for vertex bone weight buffer (per vertex)
			.addStorageBuffer(3, ShaderStage::Vertex)							// for bone transformation (per bone)
			;

		s_meshInstanceDescriptorSetLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(DescriptorSetLayout::Create(instanceDscLayoutSpec));

#pragma endregion

	}

	void VulkanMeshRenderer::CleanUp()
	{
		s_meshInstanceDescriptorSetLayout.reset();
	}

	VulkanDescriptorSetLayoutHandle VulkanMeshRenderer::GetMeshInstanceDescriptorSetLayout()
	{
		return s_meshInstanceDescriptorSetLayout;
	}

}