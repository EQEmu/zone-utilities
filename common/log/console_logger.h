#pragma once

#include "logger_interface.h"
#include <array>

namespace EQEmu
{
	class ConsoleLogger : public ILogger
	{
	public:
		ConsoleLogger();
		virtual ~ConsoleLogger();

		virtual void Enable(LogLevel level);
		virtual void Disable(LogLevel level);
		virtual bool IsEnabled(LogLevel level);
		virtual void RawLog(LogLevel level, const std::string &msg);
	private:
		std::array<bool, LogMax> _enabled;
	};
}