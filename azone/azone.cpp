#include "map.h"
#include "log_macros.h"
#include "log_stdout.h"
#include "log_file.h"

int main(int argc, char **argv) {
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogFile("azone.log")));

	for(int i = 1; i < argc; ++i) {
		Map m;
		eqLogMessage(LogInfo, "Attempting to build map for zone: %s", argv[i]);
		if(!m.Build(argv[i])) {
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
