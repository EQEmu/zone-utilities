#ifndef EQEMU_COMMON_COMPRESSION_H
#define EQEMU_COMMON_COMPRESSION_H

#include <stdint.h>

namespace eqemu
{
	namespace core
	{
		uint32_t deflate_data(const char *buffer, uint32_t len, char *out_buffer, uint32_t out_len_max);
		uint32_t inflate_data(const char* buffer, uint32_t len, char* out_buffer, uint32_t out_len_max);
	}
}

#endif