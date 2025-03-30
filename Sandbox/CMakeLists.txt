cmake_minimum_required(VERSION 3.20)


file(GLOB_RECURSE SANDBOX_SOURCES src/*.cpp src/*.h)

add_executable(Sandbox ${SANDBOX_SOURCES})

target_link_libraries(Sandbox PRIVATE PaperEngine)

# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SANDBOX_SOURCES})

# copying dll to project
if (PAPER_ENGINE_BUILD_SHARED)
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:PaperEngine>/PaperEngine.dll  # Source: DLL location
        $<TARGET_FILE_DIR:Sandbox>  # Destination: Executable folder
    )
endif()