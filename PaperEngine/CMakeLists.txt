cmake_minimum_required(VERSION 3.20)

project(PaperEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PAPER_ENGINE_SOURCES src/*.cpp src/*.h)

# target
if (PAPER_ENGINE_BUILD_SHARED)
    add_library(PaperEngine SHARED ${PAPER_ENGINE_SOURCES})
    target_compile_definitions(PaperEngine PUBLIC PE_BUILD_SHARED)


else()
    add_library(PaperEngine STATIC ${PAPER_ENGINE_SOURCES})
endif()

# define for engine building
target_compile_definitions(PaperEngine PRIVATE PE_BUILD_ITSELF)

# define debug
target_compile_definitions(PaperEngine PRIVATE "$<$<CONFIG:Debug>:PE_DEBUG>")
target_compile_definitions(PaperEngine PRIVATE "$<$<CONFIG:Release>:PE_DIST>")

# include vulkan
target_include_directories(PaperEngine PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(PaperEngine PRIVATE ${Vulkan_LIBRARIES})

# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PAPER_ENGINE_SOURCES})

target_include_directories(PaperEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# include glfw
target_link_libraries(PaperEngine PRIVATE glfw)
target_include_directories(PaperEngine PRIVATE ${glfw_SOURCE_DIR}/include)

# include spdlog
target_link_libraries(PaperEngine PUBLIC spdlog::spdlog)

