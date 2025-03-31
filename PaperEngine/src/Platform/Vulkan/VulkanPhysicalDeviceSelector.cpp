#include "VulkanPhysicalDeviceSelector.h"

#include "VulkanUtils.h"


namespace PaperEngine {



	void PaperEngine::VulkanPhysicalDeviceSelector::init(const VkInstance& instance, const VkSurfaceKHR& surface)
	{
		uint32_t num_devices = 0;

		CHECK_VK_RESULT(vkEnumeratePhysicalDevices(instance, &num_devices, NULL));

		m_devices.resize(num_devices);

		std::vector<VkPhysicalDevice> devices;
		devices.resize(num_devices);

		CHECK_VK_RESULT(vkEnumeratePhysicalDevices(instance, &num_devices, devices.data()));

		for (uint32_t i = 0; i < num_devices; i++) {

			VkPhysicalDevice phy_devices = devices[i];
			m_devices[i].physDevice = phy_devices;

			vkGetPhysicalDeviceProperties(phy_devices, &m_devices[i].properties);

			PE_CORE_TRACE("Device name: {}", m_devices[i].properties.deviceName);
			uint32_t api_version = m_devices[i].properties.apiVersion;
			PE_CORE_TRACE(
				"API version: {}.{}.{}.{}",
				VK_API_VERSION_VARIANT(api_version),
				VK_API_VERSION_MAJOR(api_version),
				VK_API_VERSION_MINOR(api_version),
				VK_API_VERSION_PATCH(api_version)
				);

			uint32_t family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(phy_devices, &family_count, NULL);
			m_devices[i].qFamilyProps.resize(family_count);
			m_devices[i].qSupportsPresent.resize(family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(phy_devices, &family_count, m_devices[i].qFamilyProps.data());

			for (uint32_t q = 0; q < family_count; q++) {
				CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(phy_devices, q, surface, &m_devices[i].qSupportsPresent[q]));
			}

			uint32_t num_formats = 0;
			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(phy_devices, surface, &num_formats, NULL));

			PE_CORE_ASSERT(num_formats > 0, "No format support in this device");
			m_devices[i].surfaceFormats.resize(num_formats);

			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(phy_devices, surface, &num_formats, m_devices[i].surfaceFormats.data()));

			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy_devices, surface, &m_devices[i].surfaceCaps));


			// present mode
			uint32_t num_present_modes = 0;
			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(phy_devices, surface, &num_present_modes, NULL));

			m_devices[i].presentModes.resize(num_present_modes);
			CHECK_VK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(phy_devices, surface, &num_present_modes, m_devices[i].presentModes.data()));

			// memory properties
			vkGetPhysicalDeviceMemoryProperties(phy_devices, &m_devices[i].memProps);

			vkGetPhysicalDeviceFeatures(phy_devices, &m_devices[i].features);
		}



	}

	uint32_t VulkanPhysicalDeviceSelector::select_device(uint32_t apiVersion, VkQueueFlags requireQueueType, bool supportPresent)
	{
		for (const auto& device : m_devices) {

			for (uint32_t i = 0; i < device.qFamilyProps.size(); i++) {
				const auto& qFamilyProp = device.qFamilyProps[i];

				if (
					(qFamilyProp.queueFlags & requireQueueType) && 
					((bool)device.qSupportsPresent[i] == supportPresent) &&
					(device.properties.apiVersion >= apiVersion)
					) {
					m_devIndex = i;
					int queueFamily = i;
					return queueFamily;
				}

			}

		}

		return 0;
	}

	const VulkanPhysicalDeviceSelector::PhysicalDevice& VulkanPhysicalDeviceSelector::get_selected() const
	{
		PE_CORE_ASSERT(m_devIndex >= 0, "There is no selected device.");

		// TODO: insert return statement here
		return m_devices[m_devIndex];
	}

}
