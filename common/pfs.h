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

#ifndef EQEMU_COMMON_PFS_HPP
#define EQEMU_COMMON_PFS_HPP

#include "archive.h"
#include <stdint.h>

namespace EQEmu
{

namespace PFS
{

namespace Internal
{

#pragma pack(1)

struct Header {
  uint32_t offset;
  char magic[4];
  uint32_t unknown;
};

struct DirectoryHeader {
  uint32_t count;
};

struct Directory {
  uint32_t crc, offset, size;
};

struct DataBlock {
  uint32_t deflate_length, inflate_length;
};

struct FilenameHeader {
  uint32_t filename_count;
};

struct FilenameEntry {
  uint32_t filename_length;
};

#pragma pack()

}

class Archive : public EQEmu::Archive
{
public:
	Archive() { }
	virtual ~Archive() { }
	
	virtual bool Open(std::string filename);
	virtual bool Close();
	virtual bool Get(std::string filename, std::vector<char> &buf);
	virtual bool Exists(std::string filename);
	virtual bool GetFilenames(std::string extension, std::list<std::string> &files);
private:
	bool ReadIntoBuffer(std::string filename);
	std::vector<std::string> filenames;
    std::vector<size_t> file_offsets;
	std::vector<char> buffer;
};

}

}

#endif
