#include "crc.h"

#include <array>
#include <memory>

#include <boost/crc.hpp>

namespace eqemu::core::detail {
    constexpr static int32_t crc_polynomial = 0x04C11DB7;
}    // namespace eqemu::core::detail

int32_t eqemu::core::crc::update(int32_t crc, const std::byte* data, size_t length) {
    // eq uses a CRC-32/CKSUM
    boost::crc_optimal<32, detail::crc_polynomial> gen(static_cast<uint32_t>(crc));
    gen.process_bytes(data, length);
    return static_cast<int32_t>(gen.checksum());
}

int32_t eqemu::core::crc::get(const std::byte* data, size_t length) {
    return update(0, data, length);
}

int32_t eqemu::core::crc::get(const std::string& s) {
    std::byte null_term = std::byte(0);
    auto ret = update(0, reinterpret_cast<const std::byte*>(s.data()), s.length());
    ret = update(ret, &null_term, 1);
    return ret;
}
