#pragma once


#include "VulkanDescriptorSetLayout.h"
#include <PaperEngine/renderer/MeshRenderer.h>
#include <PaperEngine/renderer/Material.h>
#include <PaperEngine/renderer/Mesh.h>

namespace PaperEngine {

	class VulkanMeshRenderer : public MeshRenderer {
	public:
		struct InstanceInfoUniformStruct {
			glm::mat4 transform;
			uint32_t meshType;
		};

		void prepareScene(const Scene& scene) override;

		void render(CommandBufferHandle cmd, DescriptorSetHandle globalSet, uint32_t width, uint32_t height) override;

		DescriptorSetHandle allocateInstanceSet() override;



	public:
		// intiialize in vulkan context
		static void Init();
		static void CleanUp();

		static VulkanDescriptorSetLayoutHandle GetMeshInstanceDescriptorSetLayout();

	protected:

		struct RenderCommand {
			GraphicsPipelineHandle graphicsPipeline;
			MaterialHandle material;
			DescriptorSetHandle instanceSet;
			MeshHandle mesh;
			uint32_t submeshIndex;
		};

		std::vector<RenderCommand> m_renderCommands;
	};

}
