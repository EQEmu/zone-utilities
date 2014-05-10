#include "string_util.h"

std::vector<std::string> EQEmu::SplitString(const std::string &str, char delim) {
	std::vector<std::string> ret;
	std::stringstream ss(str);
    std::string item;

    while(std::getline(ss, item, delim)) {
        ret.push_back(item);
    }
	
	return ret;
}