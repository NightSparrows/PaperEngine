#pragma once

#include "Base.h"

#include <spdlog/spdlog.h>

namespace PaperEngine {

	class Logger {
	public:
		static void Init();
		inline static Ref<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
	private:
		static Ref<spdlog::logger> s_coreLogger;
	};

}

#ifdef PE_DEBUG
#define PE_CORE_TRACE(...)    ::PaperEngine::Logger::getCoreLogger()->trace(__VA_ARGS__)
#else
#define PE_CORE_TRACE(...)
#endif
#define PE_CORE_INFO(...)     ::PaperEngine::Logger::getCoreLogger()->info(__VA_ARGS__)
#define PE_CORE_WARN(...)     ::PaperEngine::Logger::getCoreLogger()->warn(__VA_ARGS__)
#define PE_CORE_ERROR(...)    ::PaperEngine::Logger::getCoreLogger()->error(__VA_ARGS__)
#define PE_CORE_CRITICAL(...) ::PaperEngine::Logger::getCoreLogger()->critical(__VA_ARGS__)

