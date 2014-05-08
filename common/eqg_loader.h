#ifndef EQEMU_EQG_LOADER_H
#define EQEMU_EQG_LOADER_H

#include <vector>
#include <stdint.h>
#include <string>
#include "eqg_geometry.h"
#include "placeable.h"
#include "eqg_region.h"
#include "light.h"
#include "pfs.h"

class EQGLoader
{
public:
	EQGLoader();
	~EQGLoader();
	bool Load(std::string file, std::vector<Placeable> &placeables, std::vector<EQG::Region> &regions, std::vector<Light> &lights);
private:
	bool GetZon(std::string file, std::vector<char> &buffer);
	bool ParseZon(EQEmu::PFS::Archive &archive, std::vector<char> &buffer, std::vector<Placeable> &placeables, std::vector<EQG::Region> &regions, std::vector<Light> &lights);
};

#endif
