#ifndef EQEMU_COMMON_STRING_UTIL_H
#define EQEMU_COMMON_STRING_UTIL_H

#include <sstream>
#include <vector>

namespace EQEmu {
    const std::string StringFormat(const char* format, ...);
    const std::string vStringFormat(const char* format, va_list args);

    std::vector<std::string> SplitString(const std::string& str, char delim);
    bool StringsEqual(const std::string& a, const std::string& b);

}    // namespace EQEmu

#endif
