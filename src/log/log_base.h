#ifndef EQEMU_LOG_LOG_BASE_H
#define EQEMU_LOG_LOG_BASE_H

#include <string>
#include "log_types.h"

namespace EQEmu
{

namespace Log
{

class LogBase
{
public:
	LogBase() { }
	virtual ~LogBase() { }
	
	virtual void OnRegister(int enabled_logs) = 0;
	virtual void OnUnregister() = 0;
	virtual void OnMessage(LogType log_type, const std::string &message) = 0;
};

}

}

#endif