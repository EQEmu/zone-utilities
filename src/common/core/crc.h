#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace eqemu {
    namespace core {
        class crc {
        public:
            crc() = default;
            int32_t update(int32_t crc, const std::byte* data, size_t length);
            int32_t get(const std::byte* data, size_t length);
            int32_t get(const std::string& s);
        };
    }    // namespace core
}    // namespace eqemu
