#pragma once


#include <vulkan/vulkan.h>

#include <PaperEngine/core/Logger.h>

template <>
struct fmt::formatter<VkResult> : fmt::formatter<std::string> {
    auto format(VkResult result, fmt::format_context& ctx) const {
        std::string name = [result]() -> std::string {
            switch (result) {
            case VK_SUCCESS: return "VK_SUCCESS";
            case VK_NOT_READY: return "VK_NOT_READY";
            case VK_TIMEOUT: return "VK_TIMEOUT";
            case VK_EVENT_SET: return "VK_EVENT_SET";
            case VK_EVENT_RESET: return "VK_EVENT_RESET";
            case VK_INCOMPLETE: return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            default: return "Unknown VkResult (" + std::to_string(result) + ")";
            }
            }();
        return fmt::formatter<std::string>::format(name, ctx);
    }
};

#ifdef PE_DEBUG
#define CHECK_VK_RESULT(x) \
				do \
				{	\
					VkResult err = x;	\
					if (err)	\
					{				\
						PE_CORE_ERROR("Vulkan Error: {} in {}:{}", err, __FILE__, __LINE__); \
						PE_DEBUGBREAK();	\
					}	\
				} while (0) 
#else
// just call it without checking it
#define CHECK_VK_RESULT(x) x
#endif

namespace PaperEngine {

    class VulkanUtils {
    public:
        //static nvrhi::Format ConvertToNVFormat(VkFormat format);
    };

}