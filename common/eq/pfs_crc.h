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

#ifndef EQEMU_COMMON_PFS_CRC_H
#define EQEMU_COMMON_PFS_CRC_H

#include <stdint.h>
#include <string.h>
#include <string>

namespace EQEmu
{

namespace PFS
{

class CRC
{
public:
	~CRC() { }
	static CRC &Instance();
	
	int32_t Update(int32_t crc, int8_t *data, int32_t length);
	int32_t Get(std::string s);
private:
	CRC() { GenerateCRCTable(); }
	CRC(const CRC &s);
	const CRC &operator=(const CRC &s);

	void GenerateCRCTable();
	int32_t crc_table[256];
};

}

}

#endif
