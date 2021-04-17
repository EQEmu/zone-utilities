#include "config.h"
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

struct eqemu::core::config::implementation {
    json obj;
};

eqemu::core::config::config() {
    _impl = new implementation();

    std::ifstream ifs;
    ifs.open("config.json", std::ifstream::in);

    if(ifs.good()) {
        ifs >> _impl->obj;
    }
}

eqemu::core::config::~config() {
    delete _impl;
}

const std::string eqemu::core::config::get_path(const std::string& type, const std::string& default_value) {
    auto paths = _impl->obj["paths"];
    if(paths.is_object()) {
        auto map_path = paths[type];
        if(map_path.is_string()) {
            return map_path;
        }
    }

    return default_value;
}
