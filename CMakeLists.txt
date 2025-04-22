cmake_minimum_required(VERSION 3.20)

project(PaperEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# options
option(PAPER_ENGINE_BUILD_SHARED "Paper engine build shared" ON)

# get glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glfw)

# get spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
)
FetchContent_MakeAvailable(spdlog)

# get vulkan memory allocator
FetchContent_Declare(
	VMA
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG master
)
FetchContent_MakeAvailable(VMA)

# get imgui
FetchContent_Declare(
	imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master # Replace with the latest version if needed
)

FetchContent_MakeAvailable(glm)

# Fetch entt
FetchContent_Declare(
	entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG master # Replace with the latest version if needed
)
FetchContent_MakeAvailable(entt)

# find vulkan
find_package(Vulkan REQUIRED)

# Fetch NVRHI
# FetchContent_Declare(
	# nvrhi
    # GIT_REPOSITORY https://github.com/NVIDIA-RTX/NVRHI.git
    # GIT_TAG main # Replace with the latest version if needed
# )
# FetchContent_MakeAvailable(nvrhi)


# Fetch vk-bootstrap
FetchContent_Declare(
	vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG main # Replace with the latest version if needed
)
FetchContent_MakeAvailable(vk-bootstrap)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.")
endif()

add_subdirectory(PaperEngine)
add_subdirectory(Sandbox)


