#pragma once

#include <memory>

#include "PlatformDetection.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif // _WIN32


#ifdef PE_PLATFORM_WINDOWS
	#ifdef PE_BUILD_SHARED
		#define PE_API __declspec(dllexport)
	#elif PE_BUILD_STATIC
		#define PE_API 
	#else
		#define PE_API __declspec(dllimport)
	#endif
#endif

#ifdef PE_DEBUG
#if defined(PE_PLATFORM_WINDOWS)
#define PE_DEBUGBREAK() __debugbreak()
#elif defined(PE_PLATFORM_LINUX)
#include <signal.h>
#define PE_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define PE_ENABLE_ASSERTS


#else

#define PE_DEBUGBREAK()

#endif // PE_DEBUG

#define PE_EXPAND_MACRO(x) x
#define PE_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define PE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace PaperEngine {
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename U>
	Ref<T> CastRef(const Ref<U>& input) {
		return std::dynamic_pointer_cast<T>(input); // 可改 static_pointer_cast
	}

}

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/core/Assert.h>