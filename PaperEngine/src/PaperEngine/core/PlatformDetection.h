#pragma once


#ifdef _WIN32

#ifdef _WIN64
#define PE_PLATFORM_WINDOWS
#else
#error "x86 Builds are not supported!"
#endif // _WIN64

#elif defined(__APPLE__) || defined(__MACH__)
#error "Apple/MacOS is not supported!"
#elif defined(__linux__)
#define PE_PLATFORM_LINUX
#error "Linux is not supported!"
#endif // _WIN32

