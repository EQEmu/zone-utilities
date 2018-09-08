#include "composite_logger.h"

EQEmu::CompositeLogger::CompositeLogger()
{
}

EQEmu::CompositeLogger::~CompositeLogger()
{
}

void EQEmu::CompositeLogger::Add(ILogger *logger)
{
	_loggers.push_back(std::unique_ptr<ILogger>(logger));
}

void EQEmu::CompositeLogger::Enable(LogLevel level)
{
	for (auto &logger : _loggers) {
		logger->Enable(level);
	}
}

void EQEmu::CompositeLogger::Disable(LogLevel level)
{
	for (auto &logger : _loggers) {
		logger->Disable(level);
	}
}

bool EQEmu::CompositeLogger::IsEnabled(LogLevel level)
{
	for (auto &logger : _loggers) {
		if (logger->IsEnabled(level)) {
			return true;
		}
	}

	return false;
}

void EQEmu::CompositeLogger::RawLog(LogLevel level, const std::string &msg)
{
	for (auto &logger : _loggers) {
		logger->RawLog(level, msg);
	}
}
