#pragma once

#ifdef PE_PLATFORM_WINDOWS
#ifdef PE_BUILD_SHARED
#define IMGUI_API __declspec(dllexport)
#elif PE_BUILD_STATIC
#define IMGUI_API 
#else
#define IMGUI_API __declspec(dllimport)
#endif
#endif
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
//#include <ImGuizmo.h>