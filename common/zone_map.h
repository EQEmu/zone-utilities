#ifndef EQEMU_COMMON_ZONE_MAP_H
#define EQEMU_COMMON_ZONE_MAP_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm.hpp>

#define BEST_Z_INVALID -99999

class ZoneMap
{
public:
	ZoneMap();
	~ZoneMap();
	
	float FindBestZ(glm::vec3 &start, glm::vec3 *result, glm::vec3 *normal) const;
	bool Raycast(const glm::vec3 &start, const glm::vec3 &end, glm::vec3 *result, glm::vec3 *normal, float *hit_distance);
	bool LineIntersectsZone(glm::vec3 start, glm::vec3 end, float step, glm::vec3 *result) const;
	bool LineIntersectsZoneNoZLeaps(glm::vec3 start, glm::vec3 end, float step_mag, glm::vec3 *result) const;
	bool CheckLoS(glm::vec3 myloc, glm::vec3 oloc) const;
	bool Load(std::string filename);
	static ZoneMap *LoadMapFile(std::string file);
	static ZoneMap *LoadMapFromData(const std::vector<glm::vec3> &positions, const std::vector<unsigned int> &indices);
	int GetVersion();
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
