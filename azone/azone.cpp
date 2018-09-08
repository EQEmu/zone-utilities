#include "map.h"
#include <dependency/container.h>
#include <log/composite_logger.h>
#include <log/console_logger.h>
#include <log/file_logger.h>
#include <core/config.h>
#include <string.h>

void setup_dependencies() {
	auto config = EQEmu::Container::Get().RegisterSingleton<EQEmu::IConfig, EQEmu::Config>();
	auto logger = new EQEmu::CompositeLogger();

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("azone.log"));

	if (config->GetLogEnabled("Critical", true)) {
		logger->Enable(EQEmu::LogCritical);
	}

	if (config->GetLogEnabled("Error", true)) {
		logger->Enable(EQEmu::LogError);
	}

	if (config->GetLogEnabled("Debug", true)) {
		logger->Enable(EQEmu::LogDebug);
	}

	if (config->GetLogEnabled("Warning", true)) {
		logger->Enable(EQEmu::LogWarning);
	}

	if (config->GetLogEnabled("Info", true)) {
		logger->Enable(EQEmu::LogInfo);
	}

	if (config->GetLogEnabled("Trace", false)) {
		logger->Enable(EQEmu::LogTrace);
	}

	EQEmu::Container::Get().RegisterInstance<EQEmu::ILogger>(logger);
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
		logger->LogInfo("Attempting to build map for zone: {0}", argv[i]);
		if(!m.Build(argv[i], ignore_collide_tex)) {
			logger->LogError("Failed to build map for zone: {0}", argv[i]);
		} else {
			if(!m.Write(std::string(argv[i]) + std::string(".map"))) {
				logger->LogError("Failed to write map for zone {0}", argv[i]);
			} else {
				logger->LogInfo("Wrote map for zone: {0}", argv[i]);
			}
		}
	}
	
	return 0;
}
