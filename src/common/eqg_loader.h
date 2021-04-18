#ifndef EQEMU_COMMON_EQG_LOADER_H
#define EQEMU_COMMON_EQG_LOADER_H

#include "eqg_geometry.h"
#include "eqg_region.h"
#include "light.h"
#include "placeable.h"
#include <format/pfs.h>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace EQEmu {

    class EQGLoader {
    public:
        EQGLoader();
        ~EQGLoader();
        bool Load(std::string file,
                  std::vector<std::shared_ptr<EQG::Geometry>>& models,
                  std::vector<std::shared_ptr<Placeable>>& placeables,
                  std::vector<std::shared_ptr<EQG::Region>>& regions,
                  std::vector<std::shared_ptr<Light>>& lights);

    private:
        bool GetZon(std::string file, std::vector<char>& buffer);
        bool ParseZon(eqemu::format::pfs_archive& archive,
                      std::vector<char>& buffer,
                      std::vector<std::shared_ptr<EQG::Geometry>>& models,
                      std::vector<std::shared_ptr<Placeable>>& placeables,
                      std::vector<std::shared_ptr<EQG::Region>>& regions,
                      std::vector<std::shared_ptr<Light>>& lights);
    };

}    // namespace EQEmu

#endif
