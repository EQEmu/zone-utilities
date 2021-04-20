#ifndef EQEMU_COMMON_EQG_V4_LOADER_H
#define EQEMU_COMMON_EQG_V4_LOADER_H

#include "eqg_terrain.h"
#include "placeable.h"
#include "placeable_group.h"
#include <format/pfs.h>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace EQEmu {

    class EQG4Loader {
    public:
        EQG4Loader();
        ~EQG4Loader();
        bool Load(std::string file, std::shared_ptr<EQG::Terrain>& terrain);

    private:
        bool ParseZoneDat(eqemu::format::pfs_archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool ParseWaterDat(eqemu::format::pfs_archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool ParseInvwDat(eqemu::format::pfs_archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool GetZon(std::string file, std::vector<std::byte>& buffer);
        void ParseConfigFile(std::vector<std::byte>& buffer, std::vector<std::string>& tokens);
        bool ParseZon(std::vector<std::byte>& buffer, EQG::Terrain::ZoneOptions& opts);
    };

}    // namespace EQEmu

#endif
