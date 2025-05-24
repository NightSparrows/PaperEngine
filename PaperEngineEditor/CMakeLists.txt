cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE EDITOR_SOURCES src/*.cpp src/*.h)

add_executable(PaperEngineEditor ${EDITOR_SOURCES})
target_link_libraries(PaperEngineEditor PRIVATE PaperEngine)

target_link_libraries(PaperEngineEditor PRIVATE nlohmann_json::nlohmann_json)

if (PAPER_ENGINE_BUILD_SHARED)
	add_custom_command(TARGET PaperEngineEditor POST_BUILD
	  COMMAND ${CMAKE_COMMAND} 
	  -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:PaperEngineEditor>>,copy,true> 
	  $<TARGET_RUNTIME_DLLS:PaperEngineEditor> 
	  $<TARGET_FILE_DIR:PaperEngineEditor>
	  COMMAND_EXPAND_LISTS
	  )
endif()

# Visual Studio folder structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EDITOR_SOURCES})
