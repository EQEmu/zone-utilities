#ifndef EQEMU_WATER_MAP_V1_H
#define EQEMU_WATER_MAP_V1_H

#include "water_map.h"

#pragma pack(1)
typedef struct ZBSP_Node {
	int32_t node_number;
	float normal[3], splitdistance;
	int32_t region;
	int32_t special;
	int32_t left, right;
} ZBSP_Node;
#pragma pack()

class WaterMapV1 : public WaterMap
{
public:
	WaterMapV1();
	~WaterMapV1();
	
	virtual WaterRegionType ReturnRegionType(float y, float x, float z) const;
	virtual bool InWater(float y, float x, float z) const;
	virtual bool InVWater(float y, float x, float z) const;
	virtual bool InLava(float y, float x, float z) const;
	virtual bool InLiquid(float y, float x, float z) const;
protected:
	virtual bool Load(FILE *fp);

private:
	WaterRegionType BSPReturnRegionType(int32_t node_number, float y, float x, float z) const;
	ZBSP_Node* BSP_Root;

	friend class WaterMap;
};

#endif
