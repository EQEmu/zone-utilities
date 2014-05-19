#ifndef EQEMU_LOG_LOG_MACROS_H
#define EQEMU_LOG_LOG_MACROS_H

#include "log_manager.h"

#define eqLogInit(Type) EQEmu::Log::Manager::Init(Type);
#define eqLogInitAll() EQEmu::Log::Manager::Init(EQEmu::Log::LogAll);

#define eqLogRegister(obj) EQEmu::Log::Manager::Instance().RegisterLog(obj);
#define eqLogMessage(type, format, ...) EQEmu::Log::Manager::Instance().Log(EQEmu::Log::type, format, ##__VA_ARGS__);

#endif