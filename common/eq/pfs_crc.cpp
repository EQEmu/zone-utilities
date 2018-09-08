#include "pfs_crc.h"

#define polynomial 0x04C11DB7

EQEmu::PFS::CRC &EQEmu::PFS::CRC::Instance() {
	static CRC inst;
	return inst;
}

void EQEmu::PFS::CRC::GenerateCRCTable() {
	int32_t i, j, crc_accum;
	for(i = 0; i < 256; ++i) {
		crc_accum = i << 24;
		for(j = 0; j < 8; ++j) {
			if ((crc_accum & 0x80000000) != 0) {
				crc_accum = (crc_accum << 1) ^ polynomial;
			} else {
				crc_accum = crc_accum << 1;
			}
		}
		crc_table[i] = crc_accum;
	}
}

int32_t EQEmu::PFS::CRC::Update(int32_t crc, int8_t *data, int32_t length) {
	int32_t i;
	while(length > 0) {
		i = ((crc >> 24) ^ *data) & 0xFF;
		data += 1;
		crc = (crc << 8) ^ crc_table[i];
		length -= 1;
	}

	return crc;
}

int32_t EQEmu::PFS::CRC::Get(std::string s) {
	if(s.length() == 0)
		return 0;

	int8_t *buf = new int8_t[s.length() + 1];
	memset(buf, 0, s.length() + 1);
	memcpy(buf, s.c_str(), s.length());
	
	int32_t result = Update(0, buf, (int32_t)(s.length() + 1));
	delete[] buf;

	return result;
}
