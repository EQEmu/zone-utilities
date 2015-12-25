#ifndef EQEMU_WATER_MAP_V2_H
#define EQEMU_WATER_MAP_V2_H

#include "water_map.h"
#include "oriented_bounding_box.h"
#include <vector>
#include <utility>

class WaterMapV2 : public WaterMap
{
public:
	WaterMapV2();
	~WaterMapV2();

	virtual WaterRegionType ReturnRegionType(float y, float x, float z) const;
	virtual bool InWater(float y, float x, float z) const;
	virtual bool InVWater(float y, float x, float z) const;
	virtual bool InLava(float y, float x, float z) const;
	virtual bool InLiquid(float y, float x, float z) const;
	virtual int Version() const { return 2; }
	virtual void CreateMeshFrom(std::vector<glm::vec3> &verts, std::vector<unsigned int> &inds);
	virtual int GetVersion() { return 2; }

	std::vector<std::pair<WaterRegionType, OrientedBoundingBox>> &GetRegions() { return regions; }
protected:
	virtual bool Load(FILE *fp);

	std::vector<std::pair<WaterRegionType, OrientedBoundingBox>> regions;
	friend class WaterMap;
};

#endif
