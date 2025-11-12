cmake_minimum_required(VERSION 3.20)

project(PaperEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PAPER_ENGINE_SOURCES src/*.cpp src/*.h)

if (MSVC)
	# parallel compiling
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


set_target_properties(PaperEngine PROPERTIES FOLDER PaperEngine)

# enable profiling or not
if (PAPER_ENGINE_PROFILING)
	target_compile_definitions(PaperEngine PUBLIC PE_PROFILE)
endif()

# define for engine building
target_compile_definitions(PaperEngine PRIVATE PE_BUILD_ITSELF)

# define debug
target_compile_definitions(PaperEngine PRIVATE "$<$<CONFIG:Debug>:PE_DEBUG>")
target_compile_definitions(PaperEngine PRIVATE "$<$<CONFIG:Release>:PE_DIST>")

# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PAPER_ENGINE_SOURCES})

target_include_directories(PaperEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# include vulkan
target_link_libraries(PaperEngine PRIVATE ${Vulkan_LIBRARY})

# include glfw
if (PAPER_ENGINE_WINDOW_VENDOR STREQUAL "GLFW")
	target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
	target_include_directories(${PROJECT_NAME} PRIVATE ${glfw_SOURCE_DIR}/include)
endif()

# include sdl
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# include spdlog
target_link_libraries(PaperEngine PUBLIC spdlog::spdlog)

# include glm
target_link_libraries(PaperEngine PUBLIC glm::glm)

# include imgui
target_link_libraries(PaperEngine PUBLIC imgui)
#target_include_directories(PaperEngine PUBLIC ${imgui_SOURCE_DIR})

# include imguizmo
# set(IMGUIZMO_SRC_DIR vendor/ImGuizmo)
# target_include_directories(PaperEngine PUBLIC ${IMGUIZMO_SRC_DIR})

# include entt
# target_include_directories(PaperEngine PUBLIC ${entt_SOURCE_DIR}/src)
target_link_libraries(PaperEngine PUBLIC EnTT)

# include nvrhi
target_link_libraries(PaperEngine PUBLIC nvrhi)
target_link_libraries(PaperEngine PUBLIC nvrhi_vk)
target_include_directories(PaperEngine PUBLIC ${nvrhi_SOURCE_DIR}/include)

# include vk-bootstrap
target_link_libraries(PaperEngine PUBLIC vk-bootstrap::vk-bootstrap)

# include stb
target_link_libraries(PaperEngine PUBLIC stb)


# include yaml
# target_link_libraries(PaperEngine PRIVATE yaml-cpp)


# include mono
# target_include_directories(PaperEngine PRIVATE "vendor/mono/include")

#target_link_libraries(PaperEngine
#    PRIVATE
#        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/lib/Debug/libmono-static-sgen.lib>
#        $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/vendor/mono/lib/Release/libmono-static-sgen.lib>
#)

if (WIN32)
	target_link_libraries(PaperEngine PRIVATE "Ws2_32.lib")
	target_link_libraries(PaperEngine PRIVATE "Winmm.lib")
	target_link_libraries(PaperEngine PRIVATE "Version.lib")
	target_link_libraries(PaperEngine PRIVATE "Bcrypt.lib")
endif()


# copying dll to project
# if (PAPER_ENGINE_BUILD_SHARED)
	# add_custom_command(TARGET PaperEngine POST_BUILD
	  # COMMAND ${CMAKE_COMMAND} -E copy_if_different
	  # $<TARGET_RUNTIME_DLLS:Sandbox> 
	  # $<TARGET_FILE_DIR:Sandbox>
	  # COMMAND_EXPAND_LISTS
	  # )
# endif()
