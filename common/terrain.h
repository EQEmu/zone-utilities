#ifndef EQEMU_COMMON_TERRAIN_H
#define EQEMU_COMMON_TERRAIN_H

#include <memory>
#include "terrain_tile.h"
#include "water_sheet.h"

namespace EQEmu
{

class Terrain
{
public:
	Terrain() { }
	~Terrain() { }

	void AddTile(std::shared_ptr<TerrainTile> t) { tiles.push_back(t); }
	void SetQuadsPerTile(uint32_t value) { quads_per_tile = value; }
	void SetUnitsPerVertex(float value) { units_per_vertex = value; }
	void AddWaterSheet(std::shared_ptr<WaterSheet> s) { water_sheets.push_back(s); }
	
	std::vector<std::shared_ptr<TerrainTile>>& GetTiles() { return tiles; }
	uint32_t GetQuadsPerTile() { return quads_per_tile; }
	float GetUnitsPerVertex() { return units_per_vertex; }
	std::vector<std::shared_ptr<WaterSheet>>& GetWaterSheets() { return water_sheets; }

private:
	std::vector<std::shared_ptr<TerrainTile>> tiles;
	uint32_t quads_per_tile;
	float units_per_vertex;
	
	std::vector<std::shared_ptr<WaterSheet>> water_sheets;
};

}

#endif
