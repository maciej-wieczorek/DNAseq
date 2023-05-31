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

//#define ALL_LOGS
//#define NO_LOGS

#ifdef NO_LOGS
	#define LOG_TRACE(...)
	#define LOG_INFO(...)
	#define LOG_WARN(...)
	#define LOG_ERROR(...)
	#define LOG_CRITICAL(...)
#else
	#ifdef ALL_LOGS
		#define LOG_TRACE(...)    ::Logger::GetLogger()->trace(__VA_ARGS__)
		#define LOG_INFO(...)     ::Logger::GetLogger()->info(__VA_ARGS__)
		#define LOG_WARN(...)     ::Logger::GetLogger()->warn(__VA_ARGS__)
		#define LOG_ERROR(...)    ::Logger::GetLogger()->error(__VA_ARGS__)
		#define LOG_CRITICAL(...) ::Logger::GetLogger()->critical(__VA_ARGS__)
	#else
		#define LOG_TRACE(...)
		#define LOG_INFO(...)     ::Logger::GetLogger()->info(__VA_ARGS__)
		#define LOG_WARN(...)     ::Logger::GetLogger()->warn(__VA_ARGS__)
		#define LOG_ERROR(...)    ::Logger::GetLogger()->error(__VA_ARGS__)
		#define LOG_CRITICAL(...) ::Logger::GetLogger()->critical(__VA_ARGS__)
	#endif
#endif
