#ifndef EQEMU_COMMON_EQG_V4_LOADER_H
#define EQEMU_COMMON_EQG_V4_LOADER_H

#include "eqg_terrain.h"
#include "pfs.h"
#include "placeable.h"
#include "placeable_group.h"
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
        bool ParseZoneDat(EQEmu::PFS::Archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool ParseWaterDat(EQEmu::PFS::Archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool ParseInvwDat(EQEmu::PFS::Archive& archive, std::shared_ptr<EQG::Terrain>& terrain);
        bool GetZon(std::string file, std::vector<char>& buffer);
        void ParseConfigFile(std::vector<char>& buffer, std::vector<std::string>& tokens);
        bool ParseZon(std::vector<char>& buffer, EQG::Terrain::ZoneOptions& opts);
    };

}    // namespace EQEmu

#endif
