#pragma once


#include "VulkanDescriptorSetLayout.h"


namespace PaperEngine {

	class VulkanMeshRenderer {
	public:



	public:
		// intiialize in vulkan context
		static void Init();
		static void CleanUp();

		static VulkanDescriptorSetLayoutHandle GetMeshInstanceDescriptorSetLayout();

	};

}
