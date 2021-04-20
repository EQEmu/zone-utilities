#pragma once

#include <cstddef>
#include <cstdint>

namespace eqemu {
    namespace core {
        uint32_t deflate_data(const std::byte* buffer, uint32_t len, std::byte* out_buffer, uint32_t out_len_max);
        uint32_t inflate_data(const std::byte* buffer, uint32_t len, std::byte* out_buffer, uint32_t out_len_max);
    }    // namespace core
}    // namespace eqemu
