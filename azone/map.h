#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include <vector>
#include <stdint.h>
#include <string>
#include <glm.hpp>
#include "s3d_loader.h"

class Map
{
public:
	Map();
	~Map();
	
	bool Build(std::string zone_name);
	bool Write(std::string filename);
private:
	bool CompileS3D(
		std::vector<WLDFragment> &zone_frags,
		std::vector<WLDFragment> &zone_object_frags,
		std::vector<WLDFragment> &zone_light_frags,
		std::vector<WLDFragment> &object_frags,
		std::vector<WLDFragment> &character_frags
		);

	std::vector<glm::vec3> verts;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> polygon_flags;
};

#endif
