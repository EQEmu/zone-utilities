#ifndef EQEMU_COMMON_EQG_LOADER_H
#define EQEMU_COMMON_EQG_LOADER_H

#include <vector>
#include <stdint.h>
#include <string>
#include <memory>
#include "eqg_geometry.h"
#include "placeable.h"
#include "eqg_region.h"
#include "light.h"
#include "eq/pfs.h"
#include "log/logger_interface.h"

namespace EQEmu
{

class EQGLoader
{
public:
	EQGLoader();
	~EQGLoader();
	bool Load(std::string file, std::vector<std::shared_ptr<EQG::Geometry>> &models, std::vector<std::shared_ptr<Placeable>> &placeables,
		std::vector<std::shared_ptr<EQG::Region>> &regions, std::vector<std::shared_ptr<Light>> &lights);
private:
	bool GetZon(std::string file, std::vector<char> &buffer);
	bool ParseZon(EQEmu::PFS::Archive &archive, std::vector<char> &buffer, std::vector<std::shared_ptr<EQG::Geometry>> &models, std::vector<std::shared_ptr<Placeable>> &placeables,
		std::vector<std::shared_ptr<EQG::Region>> &regions, std::vector<std::shared_ptr<Light>> &lights);
	std::shared_ptr<ILogger> _logger;
};

}

#endif
