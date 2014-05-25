#include "pfs.h"
#include "pfs_crc.h"
#include "compression.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <tuple>

#define ReadFromBuffer(type, var, buffer, idx) if(idx + sizeof(type) > buffer.size()) { return false; } type var = *(type*)&buffer[idx];
#define ReadFromBufferLength(var, len, buffer, idx) if(idx + len > buffer.size()) { return false; } memcpy(var, &buffer[idx], len);

#define WriteToBuffer(type, val, buffer, idx) if(idx + sizeof(type) > buffer.size()) { buffer.resize(idx + sizeof(type)); } *(type*)&buffer[idx] = val;  
#define WriteToBufferLength(var, len, buffer, idx) if(idx + len > buffer.size()) { buffer.resize(idx + len); } memcpy(&buffer[idx], var, len);

bool EQEmu::PFS::Archive::Open() {
	Close();
	return true;
}

bool EQEmu::PFS::Archive::Open(uint32_t date) {
	Close();
	footer = true;
	footer_date = date;
	return true;
}

bool EQEmu::PFS::Archive::Open(std::string filename) {
	Close();

	std::vector<char> buffer;
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		rewind(f);

		buffer.resize(sz);
		size_t res = fread(&buffer[0], 1, sz, f);
		if (res != sz) {
			return false;
		}

		fclose(f);
	}
	else {
		return false;
	}

	char magic[4];
	ReadFromBuffer(uint32_t, dir_offset, buffer, 0);
	ReadFromBufferLength(magic, 4, buffer, 4);

	if(magic[0] != 'P' || magic[1] != 'F' || magic[2] != 'S' || magic[3] != ' ') {
		return false;
	}

	ReadFromBuffer(uint32_t, dir_count, buffer, dir_offset);
	std::vector<std::tuple<int32_t, uint32_t, uint32_t>> directory_entries;
	std::vector<std::tuple<int32_t, std::string>> filename_entries;
	for(uint32_t i = 0; i < dir_count; ++i) {
		ReadFromBuffer(int32_t, crc, buffer, dir_offset + 4 + (i * 12));
		ReadFromBuffer(uint32_t, offset, buffer, dir_offset + 8 + (i * 12));
		ReadFromBuffer(uint32_t, size, buffer, dir_offset + 12 + (i * 12));

		if (crc == 0x61580ac9) {
			std::vector<char> filename_buffer;
			if(!InflateByFileOffset(offset, size, buffer, filename_buffer)) {
				return false;
			}

			uint32_t filename_pos = 0;
			ReadFromBuffer(uint32_t, filename_count, filename_buffer, filename_pos);
			filename_pos += 4;
			for(uint32_t j = 0; j < filename_count; ++j) {
				ReadFromBuffer(uint32_t, filename_length, filename_buffer, filename_pos);
				filename_pos += 4;

				std::string filename;
				filename.resize(filename_length - 1);
				ReadFromBufferLength(&filename[0], filename_length, filename_buffer, filename_pos);
				filename_pos += filename_length;

				std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
				int32_t crc = EQEmu::PFS::CRC::Instance().Get(filename);
				filename_entries.push_back(std::make_tuple(crc, filename));
			}
		} else {
			directory_entries.push_back(std::make_tuple(crc, offset, size));
		}
	}
	
	auto iter = directory_entries.begin();
	while(iter != directory_entries.end()) {
		int32_t crc = std::get<0>((*iter));

		auto f_iter = filename_entries.begin();
		while(f_iter != filename_entries.end()) {
			int32_t f_crc = std::get<0>((*f_iter));

			if(crc == f_crc) {
				uint32_t offset = std::get<1>((*iter));
				uint32_t size = std::get<2>((*iter));
				std::string filename = std::get<1>((*f_iter));
				std::vector<char> t_buffer;

				if (!InflateByFileOffset(offset, size, buffer, t_buffer)) {
					return false;
				}

				files[filename] = t_buffer;
				break;
			}

			++f_iter;
		}
		++iter;
	}

	uint32_t footer_offset = dir_offset + 4 + (12 * dir_count);
	if (footer_offset == buffer.size()) {
		footer = false;
	} else {
		char magic[5];
		ReadFromBufferLength(magic, 5, buffer, footer_offset);
		ReadFromBuffer(uint32_t, date, buffer, footer_offset + 5);
		footer = true;
		footer_date = date;
	}

	return true;
}

bool EQEmu::PFS::Archive::Save(std::string filename) {
	std::vector<char> buffer;

	//Write Header
	WriteToBuffer(uint32_t, 0, buffer, 0);
	WriteToBuffer(uint8_t, 'P', buffer, 4);
	WriteToBuffer(uint8_t, 'F', buffer, 5);
	WriteToBuffer(uint8_t, 'S', buffer, 6);
	WriteToBuffer(uint8_t, ' ', buffer, 7);
	WriteToBuffer(uint32_t, 131072, buffer, 8);

	std::vector<std::tuple<int32_t, uint32_t, uint32_t>> dir_entries;
	std::vector<char> files_list;
	uint32_t file_offset = 0;
	uint32_t file_size = 0;
	uint32_t dir_offset = 0;
	uint32_t file_count = (uint32_t)files.size();
	uint32_t file_pos = 0;

	WriteToBuffer(uint32_t, file_count, files_list, file_pos);
	file_pos += 4;

	auto iter = files.begin();
	while(iter != files.end()) {
		int32_t crc = EQEmu::PFS::CRC::Instance().Get(iter->first);
		uint32_t offset = (uint32_t)buffer.size();
		if(!WriteDeflatedFileBlock(iter->second, buffer)) {
			return false;
		}

		dir_entries.push_back(std::make_tuple(crc, offset, (uint32_t)iter->second.size()));

		uint32_t filename_len = (uint32_t)iter->first.length() + 1;
		WriteToBuffer(uint32_t, filename_len, files_list, file_pos);
		file_pos += 4;

		WriteToBufferLength(&(iter->first[0]), filename_len - 1, files_list, file_pos);
		file_pos += filename_len;

		WriteToBuffer(uint8_t, 0, files_list, file_pos - 1);

		++iter;
	}

	file_offset = (uint32_t)buffer.size();
	if (!WriteDeflatedFileBlock(files_list, buffer)) {
		return false;
	}

	file_size = (uint32_t)files_list.size();

	dir_offset = (uint32_t)buffer.size();
	WriteToBuffer(uint32_t, dir_offset, buffer, 0);

	uint32_t cur_dir_entry_offset = dir_offset;
	uint32_t dir_count = (uint32_t)dir_entries.size() + 1;
	WriteToBuffer(uint32_t, dir_count, buffer, cur_dir_entry_offset);

	cur_dir_entry_offset += 4;
	auto dir_iter = dir_entries.begin();
	while(dir_iter != dir_entries.end())
	{
		int32_t crc = std::get<0>(*dir_iter);
		uint32_t offset = std::get<1>(*dir_iter);
		uint32_t size = std::get<2>(*dir_iter);

		WriteToBuffer(int32_t, crc, buffer, cur_dir_entry_offset);
		WriteToBuffer(uint32_t, offset, buffer, cur_dir_entry_offset + 4);
		WriteToBuffer(uint32_t, size, buffer, cur_dir_entry_offset + 8);

		cur_dir_entry_offset += 12;
		++dir_iter;
	}

	WriteToBuffer(int32_t, 0x61580AC9, buffer, cur_dir_entry_offset);
	WriteToBuffer(uint32_t, file_offset, buffer, cur_dir_entry_offset + 4);
	WriteToBuffer(uint32_t, file_size, buffer, cur_dir_entry_offset + 8);
	cur_dir_entry_offset += 12;

	if(footer) {
		WriteToBuffer(int8_t, 'S', buffer, cur_dir_entry_offset);
		WriteToBuffer(int8_t, 'T', buffer, cur_dir_entry_offset + 1);
		WriteToBuffer(int8_t, 'E', buffer, cur_dir_entry_offset + 2);
		WriteToBuffer(int8_t, 'V', buffer, cur_dir_entry_offset + 3);
		WriteToBuffer(int8_t, 'E', buffer, cur_dir_entry_offset + 4);
		WriteToBuffer(uint32_t, footer_date, buffer, cur_dir_entry_offset + 5);
	}
	
	FILE *f = fopen(filename.c_str(), "wb");
	if(f) {
		size_t sz = fwrite(&buffer[0], buffer.size(), 1, f);
		if(sz != 1) {
			fclose(f);
			return false;
		}
		fclose(f);
	} else {
		return false;
	}

	return true;
}

void EQEmu::PFS::Archive::Close() {
	footer = false;
	footer_date = 0;
	files.clear();
}

bool EQEmu::PFS::Archive::Get(std::string filename, std::vector<char> &buf) {
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

	auto iter = files.find(filename);
	if(iter != files.end()) {
		buf.clear();
		buf = iter->second;
		return true;
	}

	return false;
}

bool EQEmu::PFS::Archive::Set(std::string filename, const std::vector<char> &buf) {
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	files[filename] = buf;
	return true;
}

bool EQEmu::PFS::Archive::Delete(std::string filename) {
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

	auto iter = files.find(filename);
	if (iter != files.end()) {
		files.erase(iter);
		return true;
	}

	return false;
}

bool EQEmu::PFS::Archive::Rename(std::string filename, std::string filename_new) {
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	std::transform(filename_new.begin(), filename_new.end(), filename_new.begin(), ::tolower);

	if (files.count(filename_new) != 0) {
		return false;
	}

	auto iter = files.find(filename);
	if (iter != files.end()) {
		files[filename_new] = iter->second;
		files.erase(iter);
		return true;
	}

	return false;
}

bool EQEmu::PFS::Archive::Exists(std::string filename) {
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

	return files.count(filename) != 0;
}

bool EQEmu::PFS::Archive::GetFilenames(std::string ext, std::vector<std::string> &out_files) {
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	out_files.clear();

	size_t elen = ext.length();
	bool all_files = !ext.compare("*");

	auto iter = files.begin();
	while (iter != files.end()) {
		if (all_files) {
			out_files.push_back(iter->first);
			++iter;
			continue;
		}

		size_t flen = iter->first.length();
		if (flen <= elen)
			continue;

		if (!strcmp(iter->first.c_str() + (flen - elen), ext.c_str())) {
			out_files.push_back(iter->first);
		}
		++iter;
	}
	return out_files.size() > 0;
}

bool EQEmu::PFS::Archive::InflateByFileOffset(uint32_t offset, uint32_t size, const std::vector<char> &in_buffer, std::vector<char> &out_buffer) {
	out_buffer.resize(size);
	memset(&out_buffer[0], 0, size);

	uint32_t position = offset;
	uint32_t inflate = 0;
	while (inflate < size) {
		std::vector<char> temp_buffer;
		ReadFromBuffer(uint32_t, deflate_length, in_buffer, position);
		ReadFromBuffer(uint32_t, inflate_length, in_buffer, position + 4);
		temp_buffer.resize(deflate_length + 1);
		ReadFromBufferLength(&temp_buffer[0], deflate_length, in_buffer, position + 8);

		EQEmu::InflateData(&temp_buffer[0], deflate_length, &out_buffer[inflate], inflate_length);
		inflate += inflate_length;
		position += deflate_length + 8;
	}

	return true;
}

bool EQEmu::PFS::Archive::WriteDeflatedFileBlock(const std::vector<char> &file, std::vector<char> &out_buffer) {
	uint32_t pos = 0;
	uint32_t remain = (uint32_t)file.size();
	while(remain > 0) {
		uint32_t sz;
		if(remain >= 8192) {
			sz = 8192;
			remain -= 8192;
		} else {
			sz = remain;
			remain = 0;
		}

		uint32_t block_len = sz + 128;
		std::vector<char> block;
		block.resize(block_len);
		uint32_t deflate_size = (uint32_t)EQEmu::DeflateData(&file[pos], sz, &block[0], block_len);
		if(deflate_size == 0)
			return false;

		pos += sz;

		uint32_t idx = (uint32_t)out_buffer.size();
		WriteToBuffer(uint32_t, deflate_size, out_buffer, idx);
		WriteToBuffer(uint32_t, sz, out_buffer, idx + 4);
		out_buffer.insert(out_buffer.end(), block.begin(), block.begin() + deflate_size);
	}

	return true;
}
