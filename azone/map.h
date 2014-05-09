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
		std::vector<EQEmu::WLDFragment> &zone_frags,
		std::vector<EQEmu::WLDFragment> &zone_object_frags,
		std::vector<EQEmu::WLDFragment> &zone_light_frags,
		std::vector<EQEmu::WLDFragment> &object_frags,
		std::vector<EQEmu::WLDFragment> &character_frags
		);
	bool CompileEQG(
		std::vector<std::shared_ptr<EQEmu::EQG::Geometry>> &models,
		std::vector<std::shared_ptr<EQEmu::Placeable>> &placeables,
		std::vector<std::shared_ptr<EQEmu::EQG::Region>> &regions,
		std::vector<std::shared_ptr<EQEmu::Light>> &lights
		);

	void RotateVertex(glm::vec3 &v, float rx, float ry, float rz);
	void ScaleVertex(glm::vec3 &v, float sx, float sy, float sz);
	void TranslateVertex(glm::vec3 &v, float tx, float ty, float tz);

	std::vector<glm::vec3> collide_verts;
	std::vector<uint32_t> collide_indices;

	std::vector<glm::vec3> non_collide_verts;
	std::vector<uint32_t> non_collide_indices;
};

#endif
