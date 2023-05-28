#pragma once

#include <memory>

#include "spdlog/spdlog.h"

class Logger {

public:
	static void Init();

	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
	static std::shared_ptr<spdlog::logger> s_Logger;

};

#define LOG_TRACE(...)    ::Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)     ::Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::Logger::GetLogger()->critical(__VA_ARGS__)