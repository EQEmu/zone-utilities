#ifndef EQEMU_COMMON_PFS_ARCHIVE_HPP
#define EQEMU_COMMON_PFS_ARCHIVE_HPP

#include <stdint.h>
#include <vector>
#include <map>

namespace EQEmu
{

namespace PFS
{

class Archive
{
public:
	Archive() { }
	~Archive() { }

	bool Open();
	bool Open(uint32_t date);
	bool Open(std::string filename);
	bool Save(std::string filename);
	void Close();
	bool Get(std::string filename, std::vector<char> &buf);
	bool Set(std::string filename, const std::vector<char> &buf);
	bool Delete(std::string filename);
	bool Rename(std::string filename, std::string filename_new);
	bool Exists(std::string filename);
	bool GetFilenames(std::string ext, std::vector<std::string> &out_files);
private:
	bool InflateByFileOffset(uint32_t offset, uint32_t size, const std::vector<char> &in_buffer, std::vector<char> &out_buffer);
	bool WriteDeflatedFileBlock(const std::vector<char> &file, std::vector<char> &out_buffer);
	std::map<std::string, std::vector<char>> files;
	bool footer;
	uint32_t footer_date;
};

}

}

#endif
