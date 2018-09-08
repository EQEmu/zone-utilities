#include "logger_interface.h"
#include <stdarg.h>

//void EQEmu::ILogger::Log(LogLevel level, const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	Log(level, msg);
//}
//
//void EQEmu::ILogger::LogCritical(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogCritical(msg);
//}
//
//void EQEmu::ILogger::LogDebug(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogDebug(msg);
//}
//
//void EQEmu::ILogger::LogError(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogError(msg);
//}
//
//void EQEmu::ILogger::LogInfo(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogInfo(msg);
//}
//
//void EQEmu::ILogger::LogTrace(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogTrace(msg);
//}
//
//void EQEmu::ILogger::LogWarning(const char *str, ...)
//{
//	va_list args;
//	char buffer[32768];
//
//	va_start(args, str);
//	vsnprintf(buffer, sizeof(buffer), str, args);
//	va_end(args);
//
//	std::string msg = buffer;
//	LogWarning(msg);
//}
