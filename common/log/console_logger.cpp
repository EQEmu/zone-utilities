#include "console_logger.h"
#include <stdio.h>
#include <time.h>

EQEmu::ConsoleLogger::ConsoleLogger()
{
	for (auto &level : _enabled) {
		level = false;
	}
}

EQEmu::ConsoleLogger::~ConsoleLogger()
{
}

void EQEmu::ConsoleLogger::Enable(LogLevel level)
{
	_enabled[static_cast<int>(level)] = true;
}

void EQEmu::ConsoleLogger::Disable(LogLevel level)
{
	_enabled[static_cast<int>(level)] = false;
}

bool EQEmu::ConsoleLogger::IsEnabled(LogLevel level)
{
	return _enabled[static_cast<int>(level)];
}

void EQEmu::ConsoleLogger::RawLog(LogLevel level, const std::string & msg)
{
	if (!IsEnabled(level)) {
		return;
	}

	char time_buffer[512];
	time_t current_time;
	struct tm *time_info;

	time(&current_time);
	time_info = localtime(&current_time);

	strftime(time_buffer, 512, "[%m/%d/%y %H:%M:%S] ", time_info);

	switch (level) {
	case LogLevel::LogCritical:
		fprintf(stdout, "[CRITICAL] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogDebug:
		fprintf(stdout, "[DEBUG] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogError:
		fprintf(stdout, "[ERROR] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogInfo:
		fprintf(stdout, "[INFO] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogTrace:
		fprintf(stdout, "[TRACE] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogWarning:
		fprintf(stdout, "[WARNING] %s- %s\n", time_buffer, msg.c_str());
		break;
	}
}

//void EQEmu::ConsoleLogger::Log(LogLevel level, const std::string &msg)
//{
//	if (!IsEnabled(level)) {
//		return;
//	}
//
//	char time_buffer[512];
//	time_t current_time;
//	struct tm *time_info;
//
//	time(&current_time);
//	time_info = localtime(&current_time);
//
//	strftime(time_buffer, 512, "[%m/%d/%y %H:%M:%S] ", time_info);
//
//	switch (level) {
//	case LogLevel::LogCritical:
//		fprintf(stdout, "[CRITICAL] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	case LogLevel::LogDebug:
//		fprintf(stdout, "[DEBUG] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	case LogLevel::LogError:
//		fprintf(stdout, "[ERROR] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	case LogLevel::LogInfo:
//		fprintf(stdout, "[INFO] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	case LogLevel::LogTrace:
//		fprintf(stdout, "[TRACE] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	case LogLevel::LogWarning:
//		fprintf(stdout, "[WARNING] %s- %s\n", time_buffer, msg.c_str());
//		break;
//	}
//}
//
//void EQEmu::ConsoleLogger::LogCritical(const std::string &msg)
//{
//	Log(LogLevel::LogCritical, msg);
//}
//
//void EQEmu::ConsoleLogger::LogDebug(const std::string &msg)
//{
//	Log(LogLevel::LogDebug, msg);
//}
//
//void EQEmu::ConsoleLogger::LogError(const std::string &msg)
//{
//	Log(LogLevel::LogError, msg);
//}
//
//void EQEmu::ConsoleLogger::LogInfo(const std::string &msg)
//{
//	Log(LogLevel::LogInfo, msg);
//}
//
//void EQEmu::ConsoleLogger::LogTrace(const std::string &msg)
//{
//	Log(LogLevel::LogTrace, msg);
//}
//
//void EQEmu::ConsoleLogger::LogWarning(const std::string &msg)
//{
//	Log(LogLevel::LogWarning, msg);
//}
//