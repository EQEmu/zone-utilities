#ifndef EQEMU_COMMON_TERRAIN_H
#define EQEMU_COMMON_TERRAIN_H

#include "terrain_tile.h"
#include <memory>
#include <vector>

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

	std::vector<std::shared_ptr<TerrainTile>>& GetTiles() { return tiles; }
	uint32_t GetQuadsPerTile() { return quads_per_tile; }
	float GetUnitsPerVertex() { return units_per_vertex; }

private:
	std::vector<std::shared_ptr<TerrainTile>> tiles;
	uint32_t quads_per_tile;
	float units_per_vertex;
};

}

#endif
