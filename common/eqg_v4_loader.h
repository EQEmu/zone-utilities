#ifndef EQEMU_COMMON_EQG_V4_LOADER_H
#define EQEMU_COMMON_EQG_V4_LOADER_H

#include <vector>
#include <stdint.h>
#include <string>
#include <memory>
#include "placeable.h"
#include "placeable_group.h"
#include "terrain.h"
#include "pfs.h"

namespace EQEmu
{

class EQG4Loader
{
public:
	EQG4Loader();
	~EQG4Loader();
	bool Load(std::string file, std::shared_ptr<Terrain> &terrain);
private:
	bool ParseZoneDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain);
	bool ParseWaterDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain);
	bool ParseInvwDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain);
	bool GetZon(std::string file, std::vector<char> &buffer);
	void ParseConfigFile(std::vector<char> &buffer, std::vector<std::string> &tokens);
	bool ParseZon(std::vector<char> &buffer, Terrain::ZoneOptions &opts);
};

}

#endif
