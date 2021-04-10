#include "config.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

struct Config::Implementation {
	json obj;
};

Config::Config() {
	mImpl = new Implementation();
	
	std::ifstream ifs;
	ifs.open("config.json", std::ifstream::in);

	if (ifs.good()) {
		ifs >> mImpl->obj;
	}
}

Config::~Config() {
	delete mImpl;
}

const std::string Config::GetPath(const std::string &type, const std::string &defaultValue) {
	auto paths = mImpl->obj["paths"];
	if (paths.is_object()) {
		auto map_path = paths[type];
		if (map_path.is_string()) {
			return map_path;
		}
	}
	
	return defaultValue;
}
