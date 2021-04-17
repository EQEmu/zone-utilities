#ifndef EQEMU_COMMON_EQG_MODEL_LOADER_H
#define EQEMU_COMMON_EQG_MODEL_LOADER_H

#include "eqg_geometry.h"
#include "pfs.h"
#include <memory>
#include <stdint.h>

namespace EQEmu {

    class EQGModelLoader {
    public:
        EQGModelLoader();
        ~EQGModelLoader();
        bool Load(EQEmu::PFS::Archive& archive, std::string model, std::shared_ptr<EQG::Geometry> model_out);
    };

}    // namespace EQEmu

#endif
