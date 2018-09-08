#include "file_logger.h"
#include <time.h>

EQEmu::FileLogger::FileLogger(const std::string &filename)
{
	for (auto &level : _enabled) {
		level = false;
	}

	_filename = filename;
	_f = fopen(filename.c_str(), "wb");
}

EQEmu::FileLogger::~FileLogger()
{
	if (_f) {
		fflush(_f);
		fclose(_f);
	}
}

void EQEmu::FileLogger::Enable(LogLevel level)
{
	_enabled[static_cast<int>(level)] = true;
}

void EQEmu::FileLogger::Disable(LogLevel level)
{
	_enabled[static_cast<int>(level)] = false;
}

bool EQEmu::FileLogger::IsEnabled(LogLevel level)
{
	return _enabled[static_cast<int>(level)];
}

void EQEmu::FileLogger::RawLog(LogLevel level, const std::string & msg)
{
	if (!IsEnabled(level)) {
		return;
	}

	if (_f == nullptr) {
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
		fprintf(_f, "[CRITICAL] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogDebug:
		fprintf(_f, "[DEBUG] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogError:
		fprintf(_f, "[ERROR] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogInfo:
		fprintf(_f, "[INFO] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogTrace:
		fprintf(_f, "[TRACE] %s- %s\n", time_buffer, msg.c_str());
		break;
	case LogLevel::LogWarning:
		fprintf(_f, "[WARNING] %s- %s\n", time_buffer, msg.c_str());
		break;
	}
}
