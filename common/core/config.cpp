#include "config.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

struct EQEmu::Config::Implementation {
	json obj;
};

EQEmu::Config::Config() {
	mImpl = new Implementation();
	
	std::ifstream ifs;
	ifs.open("config.json", std::ifstream::in);

	if (ifs.good()) {
		ifs >> mImpl->obj;
	}
}

EQEmu::Config::~Config() {
	delete mImpl;
}

const std::string EQEmu::Config::GetPath(const std::string &type, const std::string &defaultValue) const {
	auto paths = mImpl->obj["paths"];
	if (paths.is_object()) {
		auto map_path = paths[type];
		if (map_path.is_string()) {
			return map_path;
		}
	}
	
	return defaultValue;
}

bool EQEmu::Config::GetLogEnabled(const std::string &type, const bool defaultValue) const
{
	auto logs = mImpl->obj["logs"];
	if (logs.is_object()) {
		auto log = logs[type];
		if (log.is_boolean()) {
			return log;
		}
	}

	return defaultValue;
}
