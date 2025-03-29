
#include "pch.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Logger.h"


namespace PaperEngine {

	Ref<spdlog::logger> Logger::s_coreLogger;
	Ref<spdlog::logger> Logger::s_clientLogger;

	void Logger::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("PaperEngine.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_coreLogger = CreateRef<spdlog::logger>("PaperEngine", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_clientLogger);
		s_coreLogger->set_level(spdlog::level::trace);
		s_coreLogger->flush_on(spdlog::level::trace);


		s_clientLogger = CreateRef<spdlog::logger>("App", begin(logSinks), end(logSinks));
		s_clientLogger->set_level(spdlog::level::trace);
		s_clientLogger->flush_on(spdlog::level::trace);
	}

}
