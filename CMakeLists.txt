cmake_minimum_required(VERSION 3.20)

option(ENABLE_DOCS "Enable documentation generation" OFF)

# window
set(PAPER_ENGINE_WINDOW_VENDOR "GLFW" CACHE STRING "Choose the window handler")

set_property(CACHE PAPER_ENGINE_WINDOW_VENDOR PROPERTY STRINGS "GLFW" "SDL")

project(PaperEngine2025)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# options
option(PAPER_ENGINE_BUILD_SHARED "Paper Engine build shared" ON)
option(PAPER_ENGINE_PROFILING "Paper Engine profiling" ON)

add_subdirectory(vendor)

add_subdirectory(PaperEngine)
add_subdirectory(Sandbox)
add_subdirectory(PaperLoader)
