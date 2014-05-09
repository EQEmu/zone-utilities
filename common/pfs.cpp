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

#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include "pfs.h"
#include "eqemu_endian.h"

#define BufferRead(x, y) x = (y*)&buffer[position]; position += sizeof(y);
#define BufferReadLength(x, y) memcpy(x, &buffer[position], y); position += y;
#define MAX_FILENAME_SIZE 1024

void decompress(const char* in, size_t in_len, char* out, size_t out_len) {
	int status;
	z_stream d_stream;

	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;

	d_stream.next_in = (Bytef*)in;
	d_stream.avail_in = (uInt)in_len;
	d_stream.next_out = (Bytef*)out;
	d_stream.avail_out = (uInt)out_len;

	inflateInit(&d_stream);
	status = inflate(&d_stream, Z_NO_FLUSH);
	inflateEnd(&d_stream);
}

bool EQEmu::PFS::Archive::Open(std::string filename)
{
	if (!ReadIntoBuffer(filename)) {
		Close();
		return false;
	}

	Internal::Header *header = nullptr;
	Internal::DirectoryHeader *directory_header = nullptr;
	Internal::Directory *directory = nullptr;
	Internal::DataBlock *data_block = nullptr;
	Internal::FilenameHeader *filename_header = nullptr;
	Internal::FilenameEntry *filename_entry = nullptr;
	size_t position = 0;

	BufferRead(header, Internal::Header);

	if (header->magic[0] != 'P' ||
		header->magic[1] != 'F' ||
		header->magic[2] != 'S' ||
		header->magic[3] != ' ')
	{
		Close();
		return false;
	}

	position = header->offset;
	BufferRead(directory_header, Internal::DirectoryHeader);

	std::vector<uint32_t> offsets(directory_header->count, 0);
	filenames.resize(directory_header->count);
	file_offsets.resize(directory_header->count);

	size_t i = 0;
	size_t j = 0;
	size_t running = 0;
	size_t temp_position = 0;
	size_t inflate = 0;
	char temp_buffer[32768];
	char temp_buffer2[32768];
	char temp_string[MAX_FILENAME_SIZE];
	for (; i < directory_header->count; ++i) {
		BufferRead(directory, Internal::Directory);
		if (directory->crc == EQEmu::NetworkToHostOrder<uint32_t>(0xC90A5861)) {
			temp_position = position;
			position = directory->offset;
			memset(temp_buffer, 0, directory->size);
			inflate = 0;

			while (inflate < directory->size) {
				BufferRead(data_block, Internal::DataBlock);
				BufferReadLength(temp_buffer2, data_block->deflate_length);
				decompress(temp_buffer2, data_block->deflate_length, temp_buffer + inflate, data_block->inflate_length);
				inflate += data_block->inflate_length;
			}

			position = temp_position;
			filename_header = (Internal::FilenameHeader*)&temp_buffer[0];
			temp_position = sizeof(Internal::FilenameHeader);

			for (j = 0; j < filename_header->filename_count; ++j)
			{
				filename_entry = (Internal::FilenameEntry*)&temp_buffer[temp_position];
				if (filename_entry->filename_length + 1 >= MAX_FILENAME_SIZE) {
					Close();
					return false;
				}
				temp_string[filename_entry->filename_length] = 0;
				memcpy(temp_string, &temp_buffer[temp_position + sizeof(Internal::FilenameEntry)], filename_entry->filename_length);
				filenames[j] = temp_string;
				temp_position += sizeof(Internal::FilenameEntry) + filename_entry->filename_length;
			}
		}
		else {
			file_offsets[running] = position - 12;
			offsets[running] = directory->offset;
			++running;
		}
	}

	uint32_t temp = 0;
	for (i = directory_header->count - 2; i > 0; i--) {
		for (j = 0; j < i; j++) {
			if (offsets[j] > offsets[j + 1]) {
				temp = offsets[j];
				offsets[j] = offsets[j + 1];
				offsets[j + 1] = temp;
				temp = (uint32_t)file_offsets[j];
				file_offsets[j] = file_offsets[j + 1];
				file_offsets[j + 1] = temp;
			}
		}
	}

	return true;
}

bool EQEmu::PFS::Archive::Close() {
	if(buffer.size() > 0) {
		filenames.resize(0);
		file_offsets.resize(0);
		buffer.resize(0);
		return true;
	}
	return false;
}

bool EQEmu::PFS::Archive::Get(std::string filename, std::vector<char> &buf) {
	size_t sz = filenames.size();
	for (size_t index = 0; index < sz; ++index) {
		if (!filenames[index].compare(filename)) {
			Internal::Directory* directory = nullptr;
			Internal::DataBlock* data_block = nullptr;
			char *temp = nullptr;

			size_t position = file_offsets[index];
			BufferRead(directory, Internal::Directory);
			position = directory->offset;

			buf.resize(directory->size);

			size_t inflate = 0;
			while (inflate < directory->size) {
				BufferRead(data_block, Internal::DataBlock);
				temp = new char[data_block->deflate_length];

				memcpy(temp, &buffer[position], data_block->deflate_length);
				position += data_block->deflate_length;

				decompress(temp, data_block->deflate_length, &buf[inflate], data_block->inflate_length);
				delete[] temp;
				inflate += data_block->inflate_length;
			}
			return true;
		}
	}

	return false;
}

bool EQEmu::PFS::Archive::Exists(std::string filename) {
	size_t count = filenames.size();
	for (size_t i = 0; i < count; ++i) {
		if (!filenames[i].compare(filename.c_str()))
			return true;
	}
	return false;
}

bool EQEmu::PFS::Archive::GetFilenames(std::string extension, std::list<std::string>& files) {
	size_t elen = extension.length();
	bool all_files = !extension.compare("*");
	files.clear();

	size_t count = filenames.size();
	for (size_t i = 0; i < count; ++i) {
		size_t flen = filenames[i].length();
		if (flen <= elen)
			continue;

		if (!strcmp(filenames[i].c_str() + (flen - elen), extension.c_str()) || all_files)
			files.push_back(filenames[i]);
	}

	return files.size() > 0;
}

bool EQEmu::PFS::Archive::ReadIntoBuffer(std::string filename) {
	FILE* f = fopen(filename.c_str(), "rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	size_t total_size = ftell(f);
	rewind(f);

	if (!total_size) {
		fclose(f);
		return false;
	}

	buffer.resize(total_size);
	size_t bytes_read = fread(&buffer[0], 1, total_size, f);

	if (bytes_read != total_size) {
		return false;
	}

	fclose(f);
	return true;
}
