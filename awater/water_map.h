#ifndef EQEMU_WATER_MAP_H
#define EQEMU_WATER_MAP_H

#include <stdint.h>
#include <string>

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
	RegionTypeVWater = 7
};

class WaterMap
{
public:
	WaterMap();
	~WaterMap();
	
	bool BuildAndWrite(std::string zone_name);
	bool BuildAndWriteS3D(std::string zone_name);
	bool BuildAndWriteEQG(std::string zone_name);
	bool BuildAndWriteEQG4(std::string zone_name);
};

#endif
