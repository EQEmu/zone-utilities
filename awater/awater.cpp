#include "water_map.h"
#include <dependency/container.h>
#include <log/composite_logger.h>
#include <log/console_logger.h>
#include <log/file_logger.h>
#include <core/config.h>

void setup_dependencies() {
	auto config = EQEmu::Container::Get().RegisterSingleton<EQEmu::IConfig, EQEmu::Config>();
	auto logger = new EQEmu::CompositeLogger();

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("awater.log"));

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

	for(int i = 1; i < argc; ++i) {
		WaterMap m;
		logger->LogInfo("Building water map for zone {0}", argv[i]);
		if(!m.BuildAndWrite(argv[i])) {
			logger->LogError("Failed to build and write water map for zone: {0}", argv[i]);
		} else {
			logger->LogInfo("Built and wrote water map for zone {0}", argv[i]);
		}
	}

	return 0;
}
