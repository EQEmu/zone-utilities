#pragma once

#include "logger_interface.h"
#include <array>
#include <stdio.h>

namespace EQEmu
{
	class FileLogger : public ILogger
	{
	public:
		FileLogger(const std::string &filename);
		virtual ~FileLogger();

		virtual void Enable(LogLevel level);
		virtual void Disable(LogLevel level);
		virtual bool IsEnabled(LogLevel level);
		virtual void RawLog(LogLevel level, const std::string &msg);

	private:
		std::array<bool, LogMax> _enabled;
		std::string _filename;
		FILE *_f;
	};
}