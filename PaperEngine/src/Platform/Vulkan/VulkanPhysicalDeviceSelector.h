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

		/// <summary>
		/// 
		/// </summary>
		/// <param name="apiVersion">
		/// mininum require vulkan api version
		/// </param>
		/// <param name="requireQueueType"></param>
		/// <param name="supportPresent"></param>
		/// <returns></returns>
		uint32_t select_device(uint32_t apiVersion, VkQueueFlags requireQueueType, bool supportPresent);

		const PhysicalDevice& get_selected() const;

	private:

		std::vector<PhysicalDevice> m_devices;

		int m_devIndex{ -1 };

	};

}
