#pragma once

#include <spdlog/spdlog.h>

namespace PaperEngine {

	class Logger {
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
		inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_clientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
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

#ifdef PE_DEBUG
#define PE_TRACE(...)         ::PaperEngine::Logger::getClientLogger()->trace(__VA_ARGS__)
#else
#define PE_TRACE(...)
#endif
#define PE_INFO(...)          ::PaperEngine::Logger::getClientLogger()->info(__VA_ARGS__)
#define PE_WARN(...)          ::PaperEngine::Logger::getClientLogger()->warn(__VA_ARGS__)
#define PE_ERROR(...)         ::PaperEngine::Logger::getClientLogger()->error(__VA_ARGS__)
#define PE_CRITICAL(...)      ::PaperEngine::Logger::getClientLogger()->critical(__VA_ARGS__)

