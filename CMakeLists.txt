cmake_minimum_required(VERSION 3.20)

option(ENABLE_DOCS "Enable documentation generation" OFF)

project(PaperEngine2025)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# Use fetch content to get submodule
include(FetchContent)

# options
option(PAPER_ENGINE_BUILD_SHARED "Paper Engine build shared" ON)
option(PAPER_ENGINE_PROFILING "Paper Engine profiling" ON)

# find vulkan
find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.")
endif()

# get glfw
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/NightSparrows/glfw
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

# get imgui
FetchContent_Declare(
	imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Fetch GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)

FetchContent_MakeAvailable(glm)

# Fetch entt
FetchContent_Declare(
	entt
    GIT_REPOSITORY https://github.com/NightSparrows/entt.git
    GIT_TAG master
)
FetchContent_MakeAvailable(entt)

set(3rdParty_folder "3rdPary")

add_subdirectory(vendor/vk-bootstrap)
set_target_properties(vk-bootstrap PROPERTIES FOLDER ${3rdParty_folder}/vk-bootstrap)
add_subdirectory(vendor/NVRHI)
set_target_properties(nvrhi PROPERTIES FOLDER ${3rdParty_folder}/NVRHI)
set_target_properties(nvrhi_vk PROPERTIES FOLDER ${3rdParty_folder}/NVRHI)
set_target_properties(nvrhi_d3d11 PROPERTIES FOLDER ${3rdParty_folder}/NVRHI)
set_target_properties(nvrhi_d3d12 PROPERTIES FOLDER ${3rdParty_folder}/NVRHI)

add_subdirectory(PaperEngine)
add_subdirectory(Sandbox)