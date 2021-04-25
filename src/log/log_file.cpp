#include "log_file.h"
#include "log_types.h"
#include <time.h>

EQEmu::Log::LogFile::LogFile(std::string file_name) {
    fp = nullptr;
    this->file_name = file_name;
}

EQEmu::Log::LogFile::~LogFile() {
    if(fp) {
        fclose(fp);
        fp = nullptr;
    }
}

void EQEmu::Log::LogFile::OnRegister(int enabled_logs) {
    if(fp) {
        fclose(fp);
        fp = nullptr;
    }

    fp = fopen(file_name.c_str(), "w+b");
}

void EQEmu::Log::LogFile::OnUnregister() {
    if(fp) {
        fclose(fp);
        fp = nullptr;
    }
}

void EQEmu::Log::LogFile::OnMessage(LogType log_type, const std::string& message) {
    if(!fp)
        return;

    char time_buffer[512];
    time_t current_time;
    struct tm* time_info;

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(time_buffer, 512, "[%m/%d/%y %H:%M:%S] ", time_info);

    switch(log_type) {
        case LogTrace: fprintf(fp, "[Trace]%s%s\n", time_buffer, message.c_str()); break;
        case LogDebug: fprintf(fp, "[Debug]%s%s\n", time_buffer, message.c_str()); break;
        case LogInfo: fprintf(fp, "[Info]%s%s\n", time_buffer, message.c_str()); break;
        case LogWarn: fprintf(fp, "[Warn]%s%s\n", time_buffer, message.c_str()); break;
        case LogError: fprintf(fp, "[Error]%s%s\n", time_buffer, message.c_str()); break;
        case LogFatal: fprintf(fp, "[Fatal]%s%s\n", time_buffer, message.c_str()); break;
        default: fprintf(fp, "[All]%s%s\n", time_buffer, message.c_str()); break;
    }

    fflush(fp);
}
