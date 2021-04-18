#ifndef EQEMU_COMMON_EQG_MODEL_LOADER_H
#define EQEMU_COMMON_EQG_MODEL_LOADER_H

#include "eqg_geometry.h"
#include <format/pfs.h>
#include <memory>
#include <stdint.h>

namespace EQEmu {

    class EQGModelLoader {
    public:
        EQGModelLoader();
        ~EQGModelLoader();
        bool Load(EQEmu::PFS::pfs_archive& archive, std::string model, std::shared_ptr<EQG::Geometry> model_out);
    };

}    // namespace EQEmu

#endif
