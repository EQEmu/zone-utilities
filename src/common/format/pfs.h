#ifndef EQEMU_COMMON_PFS_ARCHIVE_H
#define EQEMU_COMMON_PFS_ARCHIVE_H

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace EQEmu {

    namespace PFS {

        class pfs_archive {
        public:
            pfs_archive() {}
            ~pfs_archive() {}

            bool Open();
            bool Open(uint32_t date);
            bool Open(std::string filename);
            bool Save(std::string filename);
            void Close();
            bool Get(std::string filename, std::vector<char>& buf);
            bool Set(std::string filename, const std::vector<char>& buf);
            bool Delete(std::string filename);
            bool Rename(std::string filename, std::string filename_new);
            bool Exists(std::string filename);
            bool GetFilenames(std::string ext, std::vector<std::string>& out_files);

        private:
            bool StoreBlocksByFileOffset(uint32_t offset,
                                         uint32_t size,
                                         const std::vector<char>& in_buffer,
                                         std::string filename);
            bool InflateByFileOffset(uint32_t offset,
                                     uint32_t size,
                                     const std::vector<char>& in_buffer,
                                     std::vector<char>& out_buffer);
            bool WriteDeflatedFileBlock(const std::vector<char>& file, std::vector<char>& out_buffer);
            std::map<std::string, std::vector<char>> files;
            std::map<std::string, uint32_t> files_uncompressed_size;
            bool footer;
            uint32_t footer_date;
        };

    }    // namespace PFS

}    // namespace EQEmu

#endif
