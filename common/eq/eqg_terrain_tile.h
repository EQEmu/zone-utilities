#ifndef EQEMU_COMMON_EQG_TERRAIN_TILE_H
#define EQEMU_COMMON_EQG_TERRAIN_TILE_H

#include <vector>

namespace EQEmu
{

namespace  EQG 
{

class TerrainTile
{
public:
	TerrainTile() { x = y = 0.0f; flat = false; }
	~TerrainTile() { }

	void SetLocation(float nx, float ny) { x = nx; y = ny; }
	void SetFlat(bool value) { flat = value; }
	void SetBaseWaterLevel(float v) { base_water_level = v; }

	float GetX() { return x; }
	float GetY() { return y; }
	bool IsFlat() { return flat; };
	std::vector<float>& GetFloats() { return floats; }
	std::vector<uint32_t>& GetColors() { return colors; }
	std::vector<uint32_t>& GetColors2() { return colors2; }
	std::vector<uint8_t>& GetFlags() { return flags; }
	float GetBaseWaterLevel() { return base_water_level; }
private:
	float x, y;
	bool flat;
	std::vector<float> floats;
	std::vector<uint32_t> colors;
	std::vector<uint32_t> colors2;
	std::vector<uint8_t> flags;
	float base_water_level;
};

}

}

#endif
