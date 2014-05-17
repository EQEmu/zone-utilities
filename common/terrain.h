#ifndef EQEMU_COMMON_TERRAIN_H
#define EQEMU_COMMON_TERRAIN_H

#include <memory>
#include "terrain_tile.h"
#include "water_sheet.h"
#include "invis_wall.h"

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
	void AddInvisWall(std::shared_ptr<InvisWall> w) { invis_walls.push_back(w); }

	std::vector<std::shared_ptr<TerrainTile>>& GetTiles() { return tiles; }
	uint32_t GetQuadsPerTile() { return quads_per_tile; }
	float GetUnitsPerVertex() { return units_per_vertex; }
	std::vector<std::shared_ptr<WaterSheet>>& GetWaterSheets() { return water_sheets; }
	std::vector<std::shared_ptr<InvisWall>>& GetInvisWalls() { return invis_walls; }

private:
	std::vector<std::shared_ptr<TerrainTile>> tiles;
	uint32_t quads_per_tile;
	float units_per_vertex;
	
	std::vector<std::shared_ptr<WaterSheet>> water_sheets;
	std::vector<std::shared_ptr<InvisWall>> invis_walls;
};

}

#endif
