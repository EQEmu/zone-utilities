#pragma once

#include "logger_interface.h"
#include <vector>
#include <memory>

namespace EQEmu
{
	class CompositeLogger : public ILogger
	{
	public:
		CompositeLogger();
		virtual ~CompositeLogger();

		void Add(ILogger *logger);
		virtual void Enable(LogLevel level);
		virtual void Disable(LogLevel level);
		virtual bool IsEnabled(LogLevel level);
		virtual void RawLog(LogLevel, const std::string &msg);
	private:
		std::vector<std::unique_ptr<ILogger>> _loggers;
	};
}