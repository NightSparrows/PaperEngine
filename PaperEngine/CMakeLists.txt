cmake_minimum_required(VERSION 3.20)

project(PaperEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PAPER_ENGINE_SOURCES src/*.cpp src/*.h)
if (MSVC)
    add_compile_options(/MP)
endif()
# target
if (PAPER_ENGINE_BUILD_SHARED)
    add_library(PaperEngine SHARED ${PAPER_ENGINE_SOURCES})
    target_compile_definitions(PaperEngine PRIVATE PE_BUILD_SHARED)
else()
    add_library(PaperEngine STATIC ${PAPER_ENGINE_SOURCES})
    target_compile_definitions(PaperEngine PUBLIC PE_BUILD_STATIC)
endif()

# enable profiling or not
if (PAPER_ENGINE_PROFILING)
	target_compile_definitions(PaperEngine PUBLIC PE_PROFILE)
endif()

# copying dll to project
if (PAPER_ENGINE_BUILD_SHARED)
    add_custom_command(TARGET PaperEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:PaperEngine>/PaperEngine.dll  # Source: DLL location
        $<TARGET_FILE_DIR:Sandbox>  # Destination: Executable folder
    )
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

# include vulkanMemoryAllocator
target_link_libraries(PaperEngine PRIVATE VulkanMemoryAllocator)

# include glm
target_link_libraries(PaperEngine PUBLIC glm::glm)

# include imgui
target_include_directories(PaperEngine PUBLIC ${imgui_SOURCE_DIR})

# include entt
target_include_directories(PaperEngine PUBLIC ${entt_SOURCE_DIR}/src)

# include nvrhi
# target_link_libraries(PaperEngine PUBLIC nvrhi)
# target_link_libraries(PaperEngine PUBLIC nvrhi_vk)
# target_include_directories(PaperEngine PUBLIC ${nvrhi_SOURCE_DIR}/include)

# include vk-bootstrap
target_link_libraries(PaperEngine PUBLIC vk-bootstrap::vk-bootstrap)