cmake_minimum_required(VERSION 3.20)

project(PaperLoader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PAPER_LOADER_SOURCES src/*.cpp src/*.h)

if (MSVC)
	# parallel compiling
    add_compile_options(/MP)
endif()

add_library(PaperLoader STATIC ${PAPER_LOADER_SOURCES})

set_target_properties(PaperLoader PROPERTIES FOLDER PaperEngine)
# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PAPER_LOADER_SOURCES})

target_link_libraries(PaperLoader PRIVATE assimp)
target_link_libraries(PaperLoader PUBLIC PaperEngine)

