#ifndef EQEMU_COMMON_STRING_UTIL_H
#define EQEMU_COMMON_STRING_UTIL_H

#include <sstream>
#include <vector>

namespace EQEmu
{

std::vector<std::string> SplitString(const std::string &str, char delim);

}

#endif
