cmake_minimum_required(VERSION 3.20)

project(PaperEngine-ScriptCore LANGUAGES CSharp)


file(GLOB_RECURSE PAPER_ENGINE_SCRIPTCORE_SOURCES Source/*.cs)


add_library(PaperEngine-ScriptCore SHARED ${PAPER_ENGINE_SCRIPTCORE_SOURCES})

set_target_properties(PaperEngine-ScriptCore PROPERTIES FOLDER PaperEngine)