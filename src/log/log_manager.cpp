#include "log_manager.h"
#include <stdarg.h>
#include <stdio.h>

EQEmu::Log::Manager::~Manager() {
    size_t sz = logs.size();
    for(size_t i = 0; i < sz; ++i) {
        logs[i]->OnUnregister();
    }
}

void EQEmu::Log::Manager::Init(int enabled_logs) {
    auto& inst = Manager::Instance();
    inst.enabled_logs = enabled_logs;
}

EQEmu::Log::Manager& EQEmu::Log::Manager::Instance() {
    static Manager inst;
    return inst;
}

void EQEmu::Log::Manager::RegisterLog(std::shared_ptr<EQEmu::Log::LogBase> log) {
    log->OnRegister(enabled_logs);
    logs.push_back(log);
}

void EQEmu::Log::Manager::Log(LogType type, const char* str, ...) {
    if(!(enabled_logs & type)) {
        return;
    }

    va_list args;
    char buffer[32768];

    va_start(args, str);
    vsnprintf(buffer, sizeof(buffer), str, args);
    va_end(args);

    std::string msg = buffer;
    DispatchMessage(type, msg);
}

void EQEmu::Log::Manager::DispatchMessage(LogType type, const std::string& message) {
    size_t sz = logs.size();
    for(size_t i = 0; i < sz; ++i) {
        logs[i]->OnMessage(type, message);
    }
}
