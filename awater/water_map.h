#ifndef EQEMU_WATER_MAP_H
#define EQEMU_WATER_MAP_H

#include <stdint.h>
#include <string>
#include "log/logger_interface.h"

enum WaterMapRegionType
{
	RegionTypeUnsupported = -2,
	RegionTypeUntagged = -1,
	RegionTypeNormal = 0,
	RegionTypeWater = 1,
	RegionTypeLava = 2,
	RegionTypeZoneLine = 3,
	RegionTypePVP = 4,
	RegionTypeSlime = 5,
	RegionTypeIce = 6,
	RegionTypeVWater = 7,
	RegionTypeGeneralArea = 8,
	RegionTypePreferPathing = 9,
	RegionTypeDisableNavMesh = 10
};

namespace EQEmu
{
	namespace S3D {
		class BSPTree;
	}
}

class WaterMap
{
public:
	WaterMap();
	~WaterMap();
	
	bool BuildAndWrite(std::string zone_name);
	bool BuildAndWriteS3D(std::string zone_name);
	bool BuildAndWriteEQG(std::string zone_name);
	bool BuildAndWriteEQG4(std::string zone_name);
private:
	uint32_t BSPMarkRegion(std::shared_ptr<EQEmu::S3D::BSPTree> tree, uint32_t node_number, uint32_t region, int32_t region_type);
	std::shared_ptr<EQEmu::ILogger> _logger;
};

#endif
