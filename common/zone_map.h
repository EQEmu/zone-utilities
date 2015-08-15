#ifndef EQEMU_COMMON_ZONE_MAP_H
#define EQEMU_COMMON_ZONE_MAP_H

#include <vector>

#include "eq_physics.h"

class ZoneMap
{
public:
	ZoneMap();
	~ZoneMap();

	bool Load(std::string filename);
	static ZoneMap *LoadMapFile(std::string file);
	
	const std::vector<glm::vec3>& GetCollidableVerts() const;
	const std::vector<unsigned int>& GetCollidableInds() const;
	const glm::vec3& GetCollidableMax() const;
	const glm::vec3& GetCollidableMin() const;

	const std::vector<glm::vec3>& GetNonCollidableVerts() const;
	const std::vector<unsigned int>& GetNonCollidableInds() const;
	const glm::vec3& GetNonCollidableMax() const;
	const glm::vec3& GetNonCollidableMin() const;
private:
	void RotateVertex(glm::vec3 &v, float rx, float ry, float rz);
	void ScaleVertex(glm::vec3 &v, float sx, float sy, float sz);
	void TranslateVertex(glm::vec3 &v, float tx, float ty, float tz);
	bool LoadV1(FILE *f);
	bool LoadV2(FILE *f);
	
	struct impl;
	impl *imp;
};

#endif
