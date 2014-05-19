#include "map.h"
#include "log_macros.h"
#include "log_stdout.h"

int main(int argc, char **argv) {
	eqLogInit(EQEMU_LOG_LEVEL);
	std::shared_ptr<EQEmu::Log::LogBase> stdout_log(new EQEmu::Log::LogStdOut());
	eqLogRegister(stdout_log);

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
