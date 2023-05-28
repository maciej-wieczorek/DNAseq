#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init() {
	spdlog::sink_ptr logSink = std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>();
	logSink->set_pattern("%^[%T] %l: %v%$");

	s_Logger = std::make_shared<spdlog::logger>("logger", logSink);
	s_Logger->set_level(spdlog::level::trace);
	s_Logger->flush_on(spdlog::level::trace);
}