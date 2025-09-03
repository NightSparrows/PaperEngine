cmake_minimum_required(VERSION 3.20)


file(GLOB_RECURSE SANDBOX_SOURCES src/*.cpp src/*.h)

add_executable(Sandbox ${SANDBOX_SOURCES})

if (MSVC)

	# 設定working directory
	set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
endif()

target_link_libraries(Sandbox PRIVATE PaperEngine)
target_link_libraries(Sandbox PRIVATE PaperLoader)

# Group the files in Visual Studio based on folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SANDBOX_SOURCES})

	set_target_properties(Sandbox PROPERTIES FOLDER PaperEngine)