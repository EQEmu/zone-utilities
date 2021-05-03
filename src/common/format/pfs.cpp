#include <core/compression.h>
#include <core/crc.h>
#include <format/pfs.h>
#include <string/transform.h>

#include <entt/entt.hpp>
#include <spdlog/sinks/null_sink.h>

namespace eqemu::format::detail {
    constexpr size_t max_block_length = 8192;
    constexpr size_t max_block_padding = 128;
    constexpr int32_t filename_block_crc = 0x61580ac9;

    struct filename_entry {
        int32_t crc;
        std::string filename;
    };

    struct directory_entry {
        int32_t crc;
        uint32_t offset;
        uint32_t size;
    };
}    // namespace eqemu::format::detail

#define read_from_buffer(type, var, buffer, idx)                                                                       \
    if(idx + sizeof(type) > buffer.size()) {                                                                           \
        return false;                                                                                                  \
    }                                                                                                                  \
    type var = *(type*)&buffer[idx];

#define read_from_buffer_length(var, len, buffer, idx)                                                                 \
    if(idx + len > buffer.size()) {                                                                                    \
        return false;                                                                                                  \
    }                                                                                                                  \
    memcpy(var, &buffer[idx], len);

#define write_to_buffer(type, val, buffer, idx)                                                                        \
    if(idx + sizeof(type) > buffer.size()) {                                                                           \
        buffer.resize(idx + sizeof(type));                                                                             \
    }                                                                                                                  \
    *(type*)&buffer[idx] = val;

#define write_to_buffer_length(var, len, buffer, idx)                                                                  \
    if(idx + len > buffer.size()) {                                                                                    \
        buffer.resize(idx + len);                                                                                      \
    }                                                                                                                  \
    memcpy(&buffer[idx], var, len);

eqemu::format::pfs_archive::pfs_archive() : _footer(false), _footer_date(0) {
    _logger = entt::service_locator<spdlog::logger>::get().lock();

    if(nullptr == _logger) {
        _logger = spdlog::get("pfs");

        if(nullptr == _logger) {
            _logger = spdlog::null_logger_mt("pfs");
        }
    }
}

bool eqemu::format::pfs_archive::open() {
    close();
    _logger->trace("opened a blank pfs file");
    return true;
}

bool eqemu::format::pfs_archive::open(uint32_t date) {
    close();
    _footer = true;
    _footer_date = date;
    _logger->trace("opened a blank pfs file");
    return true;
}

bool eqemu::format::pfs_archive::open(const std::string& filename) {
    close();

    _logger->trace("attempting to open pfs file {0}", filename);
    std::vector<std::byte> buffer;
    FILE* f = fopen(filename.c_str(), "rb");
    if(nullptr != f) {
        fseek(f, 0, SEEK_END);
        size_t sz = ftell(f);
        rewind(f);

        buffer.resize(sz);
        size_t res = fread(&buffer[0], 1, sz, f);
        if(res != sz) {
            _logger->debug("unable to read entire pfs file {0}", filename);
            return false;
        }

        fclose(f);
    } else {
        _logger->debug("unable to open pfs file {0}", filename);
        return false;
    }

    std::array<char, 4> magic;
    read_from_buffer(uint32_t, dir_offset, buffer, 0);
    read_from_buffer_length(magic.data(), 4, buffer, 4);

    if(magic[0] != 'P' || magic[1] != 'F' || magic[2] != 'S' || magic[3] != ' ') {
        _logger->debug("magic header was corrupt for pfs file {0}", filename);
        return false;
    }

    read_from_buffer(uint32_t, dir_count, buffer, dir_offset);
    std::vector<detail::directory_entry> directory_entries;
    std::vector<detail::filename_entry> filename_entries;

    _logger->trace("pfs file {0} has {1} dir entries", filename, dir_count);

    for(uint32_t i = 0; i < dir_count; ++i) {
        read_from_buffer(int32_t, crc, buffer, dir_offset + 4 + (i * 12));
        read_from_buffer(uint32_t, offset, buffer, dir_offset + 8 + (i * 12));
        read_from_buffer(uint32_t, size, buffer, dir_offset + 12 + (i * 12));

        if(crc == detail::filename_block_crc) {
            _logger->trace("reading pfs filename dir entry");

            std::vector<std::byte> filename_buffer;
            if(!inflate_by_file_offset(offset, size, buffer, filename_buffer)) {
                _logger->debug(
                    "unable to inflate pfs filename dir entry at {0} of size {1} for file {2}", offset, size, filename);
                return false;
            }

            eqemu::core::crc crc_provider;
            uint32_t filename_pos = 0;
            read_from_buffer(uint32_t, filename_count, filename_buffer, filename_pos);
            filename_pos += 4;

            for(uint32_t j = 0; j < filename_count; ++j) {
                read_from_buffer(uint32_t, filename_length, filename_buffer, filename_pos);
                filename_pos += 4;

                std::string entry_filename;
                entry_filename.resize(filename_length - 1);
                read_from_buffer_length(&entry_filename[0], filename_length, filename_buffer, filename_pos);
                filename_pos += filename_length;

                entry_filename = eqemu::string::to_lower(entry_filename);
                int32_t crc = crc_provider.get(entry_filename);

                _logger->trace("adding filename_entry crc: {0}, entry_filename: {1}", crc, entry_filename);
                filename_entries.emplace_back(detail::filename_entry {crc, entry_filename});
            }
        } else {
            _logger->trace("adding directory_entry crc: {0}, offset: {1}, size: {2}", crc, offset, size);
            directory_entries.emplace_back(detail::directory_entry {crc, offset, size});
        }
    }

    auto iter = directory_entries.begin();
    while(iter != directory_entries.end()) {
        int32_t crc = iter->crc;

        auto f_iter = filename_entries.begin();
        while(f_iter != filename_entries.end()) {
            int32_t f_crc = f_iter->crc;

            if(crc == f_crc) {
                uint32_t offset = iter->offset;
                uint32_t size = iter->size;
                std::string ifilename = f_iter->filename;
                if(!store_blocks_by_file_offset(offset, size, buffer, ifilename)) {
                    _logger->debug("unable to store pfs chunk at {0} of size {1} for internal file {2} in pfs file {3}",
                                   offset,
                                   size,
                                   ifilename,
                                   filename);
                    return false;
                }

                break;
            }

            ++f_iter;
        }
        ++iter;
    }

    uint32_t footer_offset =
        dir_offset + sizeof(uint32_t) + ((sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint32_t)) * dir_count);

    if(footer_offset == buffer.size()) {
        _footer = false;
    } else {
        std::array<char, 5> magic;
        read_from_buffer_length(magic.data(), 5, buffer, footer_offset);
        read_from_buffer(uint32_t, date, buffer, footer_offset + 5);
        _footer = true;
        _footer_date = date;
    }

    _logger->trace("opened pfs file {0}", filename);
    return true;
}

bool eqemu::format::pfs_archive::save(const std::string& filename) {
    _logger->trace("writing pfs file {0}", filename);
    std::vector<std::byte> buffer;

    // Write Header
    write_to_buffer(uint32_t, 0, buffer, 0);
    write_to_buffer(uint8_t, 'P', buffer, 4);
    write_to_buffer(uint8_t, 'F', buffer, 5);
    write_to_buffer(uint8_t, 'S', buffer, 6);
    write_to_buffer(uint8_t, ' ', buffer, 7);
    write_to_buffer(uint32_t, 131072, buffer, 8);

    std::vector<detail::directory_entry> dir_entries;
    std::vector<std::byte> files_list;
    auto file_offset = 0U;
    auto file_size = 0U;
    auto dir_offset = 0U;
    auto file_count = (uint32_t)_files.size();
    auto file_pos = 0U;

    write_to_buffer(uint32_t, file_count, files_list, file_pos);
    file_pos += sizeof(uint32_t);

    eqemu::core::crc crc_provider;
    auto iter = _files.begin();
    while(iter != _files.end()) {
        auto crc = crc_provider.get(iter->first);
        auto offset = (uint32_t)buffer.size();
        auto sz = _files_uncompressed_size[iter->first];

        buffer.insert(buffer.end(), iter->second.begin(), iter->second.end());

        dir_entries.emplace_back(detail::directory_entry {crc, offset, sz});

        uint32_t filename_len = (uint32_t)iter->first.length() + 1;
        write_to_buffer(uint32_t, filename_len, files_list, file_pos);
        file_pos += sizeof(uint32_t);

        write_to_buffer_length(&(iter->first[0]), filename_len - 1, files_list, file_pos);
        file_pos += filename_len;

        write_to_buffer(uint8_t, 0, files_list, file_pos - 1);
        ++iter;
    }

    file_offset = (uint32_t)buffer.size();
    if(!write_deflated_file_block(files_list, buffer)) {
        _logger->debug("unable to write compressed file list block");
        return false;
    }

    file_size = (uint32_t)files_list.size();

    dir_offset = (uint32_t)buffer.size();
    write_to_buffer(uint32_t, dir_offset, buffer, 0);

    uint32_t cur_dir_entry_offset = dir_offset;
    uint32_t dir_count = (uint32_t)dir_entries.size() + 1;
    write_to_buffer(uint32_t, dir_count, buffer, cur_dir_entry_offset);

    cur_dir_entry_offset += sizeof(uint32_t);
    auto dir_iter = dir_entries.begin();
    while(dir_iter != dir_entries.end()) {
        int32_t crc = dir_iter->crc;
        uint32_t offset = dir_iter->offset;
        uint32_t size = dir_iter->size;

        write_to_buffer(int32_t, crc, buffer, cur_dir_entry_offset);
        write_to_buffer(uint32_t, offset, buffer, cur_dir_entry_offset + 4);
        write_to_buffer(uint32_t, size, buffer, cur_dir_entry_offset + 8);

        cur_dir_entry_offset += sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint32_t);
        ++dir_iter;
    }

    write_to_buffer(int32_t, detail::filename_block_crc, buffer, cur_dir_entry_offset);
    write_to_buffer(uint32_t, file_offset, buffer, cur_dir_entry_offset + 4);
    write_to_buffer(uint32_t, file_size, buffer, cur_dir_entry_offset + 8);
    cur_dir_entry_offset += sizeof(int32_t) + sizeof(uint32_t) + sizeof(uint32_t);

    if(_footer) {
        write_to_buffer(int8_t, 'S', buffer, cur_dir_entry_offset);
        write_to_buffer(int8_t, 'T', buffer, cur_dir_entry_offset + 1);
        write_to_buffer(int8_t, 'E', buffer, cur_dir_entry_offset + 2);
        write_to_buffer(int8_t, 'V', buffer, cur_dir_entry_offset + 3);
        write_to_buffer(int8_t, 'E', buffer, cur_dir_entry_offset + 4);
        write_to_buffer(uint32_t, _footer_date, buffer, cur_dir_entry_offset + sizeof(char) * 5);
    }

    FILE* f = fopen(filename.c_str(), "wb");
    if(nullptr != f) {
        size_t sz = fwrite(&buffer[0], buffer.size(), 1, f);
        if(sz != 1) {
            _logger->debug("unable to write {0} bytes to {1}", buffer.size(), filename);
            fclose(f);
            return false;
        }
        fclose(f);
    } else {
        _logger->debug("unable to open {0} for writing", filename);
        return false;
    }

    _logger->trace("wrote {0} to file", filename);
    return true;
}

void eqemu::format::pfs_archive::close() {
    _footer = false;
    _footer_date = 0;
    _files.clear();
    _files_uncompressed_size.clear();
    _logger->trace("closed pfs archive");
}

bool eqemu::format::pfs_archive::get(const std::string& filename, std::vector<std::byte>& buf) {
    auto filename_lower = eqemu::string::to_lower(filename);
    _logger->trace("attempting to get {0} from pfs archive", filename_lower);

    auto iter = _files.find(filename_lower);
    if(iter != _files.end()) {
        buf.clear();

        uint32_t uc_size = _files_uncompressed_size[filename_lower];
        if(!inflate_by_file_offset(0, uc_size, iter->second, buf)) {
            _logger->debug("unable to decompress fileblock getting {0}", filename_lower);
            return false;
        }

        _logger->trace("retrieved {0} from pfs archive", filename_lower);
        return true;
    }

    _logger->trace("unable to find {0} in pfs archive", filename_lower);
    return false;
}

bool eqemu::format::pfs_archive::set(const std::string& filename, const std::vector<std::byte>& buf) {
    auto filename_lower = eqemu::string::to_lower(filename);
    _logger->trace("attempting to set {0} in pfs archive", filename_lower);

    std::vector<std::byte> vec;
    auto uc_size = (uint32_t)buf.size();
    if(!write_deflated_file_block(buf, vec)) {
        _logger->debug("unable to compress fileblock setting {0}", filename_lower);
        return false;
    }

    _files[filename_lower] = vec;
    _files_uncompressed_size[filename_lower] = uc_size;

    _logger->trace("set {0} in pfs archive", filename_lower);
    return true;
}

bool eqemu::format::pfs_archive::remove(const std::string& filename) {
    auto filename_lower = eqemu::string::to_lower(filename);
    _logger->trace("removing {0} from pfs archive", filename_lower);

    _files.erase(filename_lower);
    _files_uncompressed_size.erase(filename_lower);

    return true;
}

bool eqemu::format::pfs_archive::rename(const std::string& filename, const std::string& filename_new) {
    auto filename_lower = eqemu::string::to_lower(filename);
    auto filename_new_lower = eqemu::string::to_lower(filename_new);
    _logger->trace("renaming {0} to {1} in pfs archive", filename_lower, filename_new_lower);

    if(_files.count(filename_new_lower) != 0) {
        _logger->debug("unable to rename {0} to {1} because {1} already exists in pfs archive",
                       filename_lower,
                       filename_new_lower);
        return false;
    }

    auto iter = _files.find(filename_lower);
    if(iter != _files.end()) {
        _files[filename_new_lower] = iter->second;
        _files.erase(iter);

        auto iter_s = _files_uncompressed_size.find(filename_lower);
        _files_uncompressed_size[filename_new_lower] = iter_s->second;
        _files_uncompressed_size.erase(iter_s);
        _logger->trace("renamed {0} to {1} in pfs archive", filename_lower, filename_new_lower);
        return true;
    }

    _logger->debug(
        "unable to rename {0} to {1} because {0} did not exist in pfs archive", filename_lower, filename_new_lower);
    return false;
}

bool eqemu::format::pfs_archive::exists(const std::string& filename) {
    auto filename_lower = eqemu::string::to_lower(filename);

    return _files.count(filename_lower) != 0;
}

bool eqemu::format::pfs_archive::get_filenames(const std::string& ext, std::vector<std::string>& out_files) {
    auto ext_lower = eqemu::string::to_lower(ext);
    out_files.clear();

    size_t elen = ext_lower.length();
    bool all_files = ext_lower != "*";

    auto iter = _files.begin();
    while(iter != _files.end()) {
        if(all_files) {
            out_files.push_back(iter->first);
            ++iter;
            continue;
        }

        size_t flen = iter->first.length();
        if(flen <= elen) {
            ++iter;
            continue;
        }

        if(!strcmp(iter->first.c_str() + (flen - elen), ext_lower.c_str())) {
            out_files.push_back(iter->first);
        }
        ++iter;
    }

    return false == out_files.empty();
}

bool eqemu::format::pfs_archive::store_blocks_by_file_offset(uint32_t offset,
                                                             uint32_t size,
                                                             const std::vector<std::byte>& in_buffer,
                                                             const std::string& filename) {
    auto position = offset;
    auto block_size = 0U;
    auto inflate = 0U;
    while(inflate < size) {
        read_from_buffer(uint32_t, deflate_length, in_buffer, position);
        read_from_buffer(uint32_t, inflate_length, in_buffer, position + 4);
        inflate += inflate_length;
        position += deflate_length + sizeof(uint32_t) + sizeof(uint32_t);
    }

    block_size = position - offset;

    std::vector<std::byte> tbuffer;
    tbuffer.resize(block_size);

    memcpy(&tbuffer[0], &in_buffer[offset], block_size);

    _files[filename] = tbuffer;
    _files_uncompressed_size[filename] = size;
    return true;
}

bool eqemu::format::pfs_archive::inflate_by_file_offset(uint32_t offset,
                                                        uint32_t size,
                                                        const std::vector<std::byte>& in_buffer,
                                                        std::vector<std::byte>& out_buffer) {
    out_buffer.resize(size);
    memset(&out_buffer[0], 0, size);

    auto position = offset;
    auto inflate = 0U;

    while(inflate < size) {
        std::vector<std::byte> temp_buffer;
        read_from_buffer(uint32_t, deflate_length, in_buffer, position);
        read_from_buffer(uint32_t, inflate_length, in_buffer, position + 4);
        temp_buffer.resize(deflate_length + 1);
        read_from_buffer_length(&temp_buffer[0], deflate_length, in_buffer, position + 8);

        eqemu::core::inflate_data(&temp_buffer[0], deflate_length, &out_buffer[inflate], inflate_length);
        inflate += inflate_length;
        position += deflate_length + sizeof(uint32_t) + sizeof(uint32_t);
    }

    return true;
}

bool eqemu::format::pfs_archive::write_deflated_file_block(const std::vector<std::byte>& file,
                                                           std::vector<std::byte>& out_buffer) {
    auto pos = 0U;
    auto remain = (uint32_t)file.size();
    std::array<std::byte, detail::max_block_length + detail::max_block_padding> block;

    while(remain > 0) {
        uint32_t sz;
        if(remain >= detail::max_block_length) {
            sz = detail::max_block_length;
            remain -= detail::max_block_length;
        } else {
            sz = remain;
            remain = 0;
        }

        auto block_len = (uint32_t)(sz + detail::max_block_padding);
        auto deflate_size = (uint32_t)eqemu::core::deflate_data(&file[pos], sz, (std::byte*)&block[0], block_len);
        if(0 == deflate_size) {
            return false;
        }

        pos += sz;

        auto idx = (uint32_t)out_buffer.size();
        write_to_buffer(uint32_t, deflate_size, out_buffer, idx);
        write_to_buffer(uint32_t, sz, out_buffer, idx + 4);
        out_buffer.insert(out_buffer.end(), &block[0], &block[deflate_size]);
    }

    return true;
}
