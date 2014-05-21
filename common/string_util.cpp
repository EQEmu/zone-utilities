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

bool EQEmu::StringsEqual(const std::string& a, const std::string& b)
{
	size_t sz = a.size();
	if (b.size() != sz) {
		return false;
	}

	for (unsigned int i = 0; i < sz; ++i) {
		if (tolower(a[i]) != tolower(b[i])) {
			return false;
		}
	}

	return true;
}