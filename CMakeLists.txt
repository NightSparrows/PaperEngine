cmake_minimum_required(VERSION 3.20)

option(ENABLE_DOCS "Enable documentation generation" OFF)

project(PaperEngine2025)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# options
option(PAPER_ENGINE_BUILD_SHARED "Paper Engine build shared" ON)
option(PAPER_ENGINE_PROFILING "Paper Engine profiling" ON)

# find vulkan
find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found.")
endif()

add_subdirectory(vendor)

add_subdirectory(PaperEngine)
add_subdirectory(Sandbox)