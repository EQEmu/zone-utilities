#include "water_map.h"
#include "dependency/container.h"
#include "log/composite_logger.h"
#include "log/console_logger.h"
#include "log/file_logger.h"

void setup_dependencies() {
	EQEmu::Container::Get().RegisterSingleton<EQEmu::ILogger, EQEmu::CompositeLogger>();
	auto logger = std::dynamic_pointer_cast<EQEmu::CompositeLogger>(EQEmu::Container::Get().Resolve<EQEmu::ILogger>());

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("awater.log"));
	logger->Enable(EQEmu::LogCritical);
	logger->Enable(EQEmu::LogError);
	logger->Enable(EQEmu::LogDebug);
	logger->Enable(EQEmu::LogWarning);
	logger->Enable(EQEmu::LogInfo);
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
