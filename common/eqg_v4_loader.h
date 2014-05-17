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
	struct ZoneOptions {
		std::string name;
		int32_t min_lng;
		int32_t max_lng;
		int32_t min_lat;
		int32_t max_lat;
		float min_extents[3];
		float max_extents[3];
		float units_per_vert;
		uint32_t quads_per_tile;
		uint32_t cover_map_input_size;
		uint32_t layer_map_input_size;
	};

public:
	EQG4Loader();
	~EQG4Loader();
	bool Load(std::string file, std::shared_ptr<Terrain> &terrain);
private:
	bool ParseZoneDat(EQEmu::PFS::Archive &archive, ZoneOptions &opts, std::shared_ptr<Terrain> &terrain);
	bool ParseWaterDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain);
	bool ParseInvwDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain);
	bool GetZon(std::string file, std::vector<char> &buffer);
	void ParseConfigFile(std::vector<char> &buffer, std::vector<std::string> &tokens);
	bool ParseZon(std::vector<char> &buffer, ZoneOptions &opts);
};

}

#endif
