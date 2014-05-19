#include <stdio.h>
#include "water_map.h"
#include "log_macros.h"
#include "log_stdout.h"

int main(int argc, char **argv) {
	eqLogInit(EQEMU_LOG_LEVEL);
	std::shared_ptr<EQEmu::Log::LogBase> stdout_log(new EQEmu::Log::LogStdOut());
	eqLogRegister(stdout_log);

	for(int i = 1; i < argc; ++i) {
		WaterMap m;
		eqLogMessage(LogInfo, "Building water map for zone %s", argv[i]);
		if(!m.BuildAndWrite(argv[i])) {
			printf("Failed to build and write water map for zone: %s\n", argv[i]);
			eqLogMessage(LogError, "Failed to build and write water map for zone: %s", argv[i]);
		} else {
			eqLogMessage(LogInfo, "Built and wrote water map for zone %s", argv[i]);
		}
	}

	return 0;
}
