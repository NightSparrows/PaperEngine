cmake_minimum_required(VERSION 3.20)


file(GLOB_RECURSE SANDBOX_SOURCES src/*.cpp src/*.h)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE PaperEngine)

# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SANDBOX_SOURCES})
