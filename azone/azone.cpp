#include "map.h"
#include "dependency/container.h"
#include "log/composite_logger.h"
#include "log/console_logger.h"
#include "log/file_logger.h"
#include <string.h>

void setup_dependencies() {
	EQEmu::Container::Get().RegisterSingleton<EQEmu::ILogger, EQEmu::CompositeLogger>();
	auto logger = std::dynamic_pointer_cast<EQEmu::CompositeLogger>(EQEmu::Container::Get().Resolve<EQEmu::ILogger>());

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("azone.log"));
	logger->Enable(EQEmu::LogCritical);
	logger->Enable(EQEmu::LogError);
	logger->Enable(EQEmu::LogDebug);
	logger->Enable(EQEmu::LogWarning);
	logger->Enable(EQEmu::LogInfo);
}

int main(int argc, char **argv) {
	setup_dependencies();
	
	auto logger = EQEmu::Container::Get().Resolve<EQEmu::ILogger>();

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
		logger->LogInfo("Attempting to build map for zone: %s", argv[i]);
		if(!m.Build(argv[i], ignore_collide_tex)) {
			logger->LogError("Failed to build map for zone: %s", argv[i]);
		} else {
			if(!m.Write(std::string(argv[i]) + std::string(".map"))) {
				logger->LogError("Failed to write map for zone %s", argv[i]);
			} else {
				logger->LogInfo("Wrote map for zone: %s", argv[i]);
			}
		}
	}
	
	return 0;
}
