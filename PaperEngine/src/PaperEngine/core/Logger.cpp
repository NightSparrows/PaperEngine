
#include "Logger.h"
#include <iostream>

#ifdef _MSVC_VER
#pragma warning(push)
#pragma warning(disable : 6294)
#pragma warning(disable : 26495)
#pragma warning(disable : 26498)
#endif
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#ifdef _MSVC_VER
#pragma warning(pop)
#endif


namespace PaperEngine {

	Ref<spdlog::logger> Logger::s_coreLogger;

	void Logger::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("PaperEngine.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_coreLogger = CreateRef<spdlog::logger>("PaperEngine", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_coreLogger);
		s_coreLogger->set_level(spdlog::level::trace);
		s_coreLogger->flush_on(spdlog::level::trace);
	}

}
