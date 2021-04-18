#ifndef EQEMU_COMMON_PFS_ARCHIVE_H
#define EQEMU_COMMON_PFS_ARCHIVE_H

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

namespace eqemu {

    namespace format {

        class pfs_archive {
        public:
            pfs_archive() {}
            ~pfs_archive() {}

            bool open();
            bool open(uint32_t date);
            bool open(const std::string& filename);
            bool save(const std::string& filename);
            void close();
            bool get(const std::string& filename, std::vector<char>& buf);
            bool set(const std::string& filename, const std::vector<char>& buf);
            bool remove(const std::string& filename);
            bool rename(const std::string& filename, const std::string& filename_new);
            bool exists(const std::string& filename);
            bool get_filenames(const std::string& ext, std::vector<std::string>& out_files);

        private:
            bool store_blocks_by_file_offset(uint32_t offset,
                                             uint32_t size,
                                             const std::vector<char>& in_buffer,
                                             const std::string& filename);
            bool inflate_by_file_offset(uint32_t offset,
                                        uint32_t size,
                                        const std::vector<char>& in_buffer,
                                        std::vector<char>& out_buffer);
            bool write_deflated_file_block(const std::vector<char>& file, std::vector<char>& out_buffer);
            std::map<std::string, std::vector<char>> _files;
            std::map<std::string, uint32_t> _files_uncompressed_size;
            bool _footer;
            uint32_t _footer_date;
        };

    }    // namespace format

}    // namespace eqemu

#endif
