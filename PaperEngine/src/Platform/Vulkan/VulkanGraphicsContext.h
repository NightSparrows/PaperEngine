#pragma once


#include <PaperEngine/graphics/GraphicsContext.h>
#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>

namespace PaperEngine {

	struct VulkanInstance {

		VkDevice vulkanDevice;
		VkPhysicalDevice vkPhysicalDevice;
		VkQueue graphicsQueue;


		nvrhi::DeviceHandle device;

	};

	class VulkanGraphicsContext : public GraphicsContext {
	public:
		void init() override;

		void cleanUp() override;



	};
}
