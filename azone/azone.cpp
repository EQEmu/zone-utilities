#include "map.h"
#include "log_macros.h"
#include "log_stdout.h"
#include "log_file.h"
#include <string.h>

int main(int argc, char **argv) {
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogFile("azone.log")));

	int i = 1;
	bool ignore_collide_tex = true;
	if (argc > 2) {
		if (strcmp(argv[1], "--IncludeCollideTex") == 0) {
			ignore_collide_tex = false;
			i = 2;
		}
	}

	for(; i < argc; ++i) {
		Map m;
		eqLogMessage(LogInfo, "Attempting to build map for zone: %s", argv[i]);
		if(!m.Build(argv[i], ignore_collide_tex)) {
			eqLogMessage(LogError, "Failed to build map for zone: %s", argv[i]);
		} else {
			if(!m.Write(std::string(argv[i]) + std::string(".map"))) {
				eqLogMessage(LogError, "Failed to write map for zone %s", argv[i]);
			} else {
				eqLogMessage(LogInfo, "Wrote map for zone: %s", argv[i]);
			}
		}
	}

	return 0;
}
