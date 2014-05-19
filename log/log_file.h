#ifndef EQEMU_LOG_LOG_FILE_H
#define EQEMU_LOG_LOG_FILE_H

#include <string>
#include <stdio.h>
#include "log_base.h"

namespace EQEmu
{

namespace Log
{

class LogFile : public LogBase
{
public:
	LogFile(std::string file_name);
	virtual ~LogFile();
	
	virtual void OnRegister(int enabled_logs);
	virtual void OnUnregister();
	virtual void OnMessage(LogType log_types, const std::string &message);
	
private:
	FILE *fp;
	std::string file_name;
};

}

}

#endif