#include "VulkanMeshRenderer.h"

namespace PaperEngine {

	static VulkanDescriptorSetLayoutHandle s_meshInstanceDescriptorSetLayout;

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