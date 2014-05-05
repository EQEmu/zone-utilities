#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include <vector>
#include <stdint.h>
#include <string>

class Map
{
public:
	struct Vertex
	{
		float x, y, z;
	};
	
	Map();
	~Map();
	
	bool Build(std::string zone_name);
	bool Write(std::string filename);
private:
	std::vector<Vertex> verts;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> polygon_flags;
};

#endif
