#include "log_stdout.h"
#include "log_types.h"
#include <stdio.h>
#include <time.h>

void EQEmu::Log::LogStdOut::OnMessage(LogType log_type, const std::string &message) {
	char time_buffer[512];
	time_t current_time;
	struct tm *time_info;
	
	time(&current_time);
	time_info = localtime(&current_time);
	
	strftime(time_buffer, 512, "[%m/%d/%y %H:%M:%S] ", time_info);
	
	switch(log_type) {
	case LogTrace:
		fprintf(stdout, "[Trace]%s%s\n", time_buffer, message.c_str());
		break;
	case LogDebug:
		fprintf(stdout, "[Debug]%s%s\n", time_buffer, message.c_str());
		break;
	case LogInfo:
		fprintf(stdout, "[Info]%s%s\n", time_buffer, message.c_str());
		break;
	case LogWarn:
		fprintf(stdout, "[Warn]%s%s\n", time_buffer, message.c_str());
		break;
	case LogError:
		fprintf(stdout, "[Error]%s%s\n", time_buffer, message.c_str());
		break;
	case LogFatal:
		fprintf(stdout, "[Fatal]%s%s\n", time_buffer, message.c_str());
		break;
	default:
		fprintf(stdout, "[All]%s%s\n", time_buffer, message.c_str());
		break;
	}
}
