#include "config.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

struct eqemu::config::implementation {
	json obj;
};

eqemu::config::config() {
	_impl = new implementation();
	
	std::ifstream ifs;
	ifs.open("config.json", std::ifstream::in);

	if (ifs.good()) {
		ifs >> _impl->obj;
	}
}

eqemu::config::~config() {
	delete _impl;
}

const std::string eqemu::config::get_path(const std::string &type, const std::string &defaultValue) {
	auto paths = _impl->obj["paths"];
	if (paths.is_object()) {
		auto map_path = paths[type];
		if (map_path.is_string()) {
			return map_path;
		}
	}
	
	return defaultValue;
}
