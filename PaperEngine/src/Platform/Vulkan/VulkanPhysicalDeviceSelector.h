#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace PaperEngine {

	class VulkanPhysicalDeviceSelector {
	public:
		
		struct PhysicalDevice {
			VkPhysicalDevice physDevice{ VK_NULL_HANDLE };
			VkPhysicalDeviceProperties properties;
			std::vector<VkQueueFamilyProperties> qFamilyProps;
			std::vector<VkBool32> qSupportsPresent;
			std::vector<VkSurfaceFormatKHR> surfaceFormats;
			VkSurfaceCapabilitiesKHR surfaceCaps;
			VkPhysicalDeviceMemoryProperties memProps;
			std::vector<VkPresentModeKHR> presentModes;
			VkPhysicalDeviceFeatures features;
		};

	public:

		// fetch the deivces
		void init(const VkInstance& instance, const VkSurfaceKHR& surface);

		uint32_t select_device(VkQueueFlags requireQueueType, bool supportPresent);

		const PhysicalDevice& get_selected() const;

	private:

		std::vector<PhysicalDevice> m_devices;

		int m_devIndex{ -1 };

	};

}
