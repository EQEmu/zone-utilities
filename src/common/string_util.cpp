/*
 * Copyright 2013 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef WIN32
#    include <windows.h>

#    if defined(_MSC_VER) && (_MSC_VER < 1900)
#        define snprintf    _snprintf
#        define strncasecmp _strnicmp
#        define strcasecmp  _stricmp
#    endif
#else
#    include <stdio.h>
#    include <stdlib.h>
#endif

#include <stdarg.h>

#ifndef va_copy
#    define va_copy(d, s) ((d) = (s))
#endif

#include "string_util.h"

// original source:
// https://github.com/facebook/folly/blob/master/folly/String.cpp
//
const std::string EQEmu::vStringFormat(const char* format, va_list args) {
    std::string output;
    va_list tmpargs;

    va_copy(tmpargs, args);
    int characters_used = vsnprintf(nullptr, 0, format, tmpargs);
    va_end(tmpargs);

    // Looks like we have a valid format string.
    if(characters_used > 0) {
        output.resize(characters_used + 1);

        va_copy(tmpargs, args);
        characters_used = vsnprintf(&output[0], output.capacity(), format, tmpargs);
        va_end(tmpargs);

        output.resize(characters_used);

        // We shouldn't have a format error by this point, but I can't imagine what error we
        // could have by this point. Still, return empty string;
        if(characters_used < 0)
            output.clear();
    }
    return output;
}

const std::string EQEmu::StringFormat(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::string output = vStringFormat(format, args);
    va_end(args);
    return output;
}

std::vector<std::string> EQEmu::SplitString(const std::string& str, char delim) {
    std::vector<std::string> ret;
    std::stringstream ss(str);
    std::string item;

    while(std::getline(ss, item, delim)) {
        ret.push_back(item);
    }

    return ret;
}

bool EQEmu::StringsEqual(const std::string& a, const std::string& b) {
    size_t sz = a.size();
    if(b.size() != sz) {
        return false;
    }

    for(unsigned int i = 0; i < sz; ++i) {
        if(tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }

    return true;
}
