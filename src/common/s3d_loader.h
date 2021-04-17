#ifndef EQEMU_S3D_LOADER_H
#define EQEMU_S3D_LOADER_H

#include "wld_fragment.h"
#include <stdint.h>
#include <string>
#include <vector>

void decode_string_hash(char* str, size_t len);

namespace EQEmu {

    class S3DLoader {
    public:
        S3DLoader();
        ~S3DLoader();
        bool ParseWLDFile(std::string file_name, std::string wld_name, std::vector<S3D::WLDFragment>& out);
    };

}    // namespace EQEmu

#endif
