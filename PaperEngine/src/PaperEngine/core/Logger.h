#pragma once

#include "Base.h"

#pragma warning(push)
#pragma warning(disable : 6294)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#include <spdlog/spdlog.h>
#pragma warning(pop)
namespace PaperEngine {

	class Logger {
	public:
		PE_API static void Init();
		PE_API inline static Ref<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }
	private:
		static Ref<spdlog::logger> s_coreLogger;
	};

}

#ifdef PE_DEBUG
#define PE_CORE_TRACE(...)    ::PaperEngine::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#else
#define PE_CORE_TRACE(...)
#endif
#define PE_CORE_INFO(...)     ::PaperEngine::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define PE_CORE_WARN(...)     ::PaperEngine::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define PE_CORE_ERROR(...)    ::PaperEngine::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define PE_CORE_CRITICAL(...) ::PaperEngine::Logger::GetCoreLogger()->critical(__VA_ARGS__)

