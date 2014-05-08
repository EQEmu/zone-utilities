#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include <vector>
#include <stdint.h>
#include <string>
#include <glm.hpp>
#include "s3d_loader.h"
#include "eqg_loader.h"

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
	bool CompileEQG(
		std::vector<Placeable> &placeables,
		std::vector<EQG::Region> &regions,
		std::vector<Light> &lights
		);

	void RotateVertex(Geometry::Vertex &v, float rx, float ry, float rz);
	void ScaleVertex(Geometry::Vertex &v, float sx, float sy, float sz);
	void TranslateVertex(Geometry::Vertex &v, float tx, float ty, float tz);

	std::vector<glm::vec3> collide_verts;
	std::vector<uint32_t> collide_indices;

	std::vector<glm::vec3> non_collide_verts;
	std::vector<uint32_t> non_collide_indices;
};

#endif
