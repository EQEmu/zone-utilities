/*
    Copyright(C) 2014 EQEmu

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef EQEMU_COMMON_ENDIAN_H
#define EQEMU_COMMON_ENDIAN_H

namespace EQEmu {

    template <typename T>
    static inline T NetworkToHostOrder(T v) {
        uint32_t one = 1;
        char test = *(char*)&one;
        if(test == 0) {
            return v;
        }

        T r;
        size_t sz = sizeof(T);
        char* src = (char*)&v + sizeof(T) - 1;
        char* dst = (char*)&r;

        for(; sz-- > 0;) {
            *dst++ = *src--;
        }

        return r;
    }

    template <typename T>
    static inline T HostToNetworkOrder(T v) {
        uint32_t one = 1;
        char test = *(char*)&one;
        if(test == 0) {
            return v;
        }

        T r;
        size_t sz = sizeof(T);
        char* src = (char*)&v + sizeof(T) - 1;
        char* dst = (char*)&r;

        for(; sz-- > 0;) {
            *dst++ = *src--;
        }

        return r;
    }

}    // namespace EQEmu

#endif
