#ifndef EQEMU_LOG_LOG_STDOUT_H
#define EQEMU_LOG_LOG_STDOUT_H

#include <string>
#include "log_base.h"

namespace EQEmu
{

namespace Log
{

class LogStdOut : public LogBase
{
public:
	LogStdOut() { }
	virtual ~LogStdOut() { }
	
	virtual void OnRegister(int enabled_logs) { }
	virtual void OnUnregister() { }
	virtual void OnMessage(LogType log_types, const std::string &message);
};

}

}

#endif