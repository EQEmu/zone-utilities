#include "transform.h"

#include <algorithm>

std::string eqemu::string::to_lower(const std::string& str) {
    std::string ret = str;
    std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
    return ret;
}

std::string eqemu::string::to_upper(const std::string& str) {
    std::string ret = str;
    std::transform(str.begin(), str.end(), ret.begin(), ::toupper);
    return ret;
}
