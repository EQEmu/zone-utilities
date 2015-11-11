#ifndef EQEMU_COMMON_EQ_PHYSICS_H
#define EQEMU_COMMON_EQ_PHYSICS_H

#include <vector>

#include "oriented_bounding_box.h"
#include "water_map.h"

#define BEST_FLOOR_INVALID -9999999
#define BEST_CEIL_INVALID 9999999

enum EQPhysicsFlags
{
	CollidableWorld = 1,
	NonCollidableWorld = 2,
	Selectable = 4
};

class EQPhysics
{
public:
	EQPhysics();
	~EQPhysics();
	
	//manipulation
	void SetCollidableWorld(const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds);
	void SetNonCollidableWorld(const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds);
	void SetWaterMap(WaterMap *w);

	void Step();

	//collision stuff
	bool CheckLOS(const glm::vec3 &src, const glm::vec3 &dest) const;
	bool GetRaycastClosestHit(const glm::vec3 &src, const glm::vec3 &dest, glm::vec3 &hit, EQPhysicsFlags flag = CollidableWorld) const;
	float FindBestFloor(const glm::vec3 &start, glm::vec3 *result, glm::vec3 *normal) const;
	bool IsUnderworld(const glm::vec3 &point) const;
	bool CheckLosNoHazards(const glm::vec3 &start, const glm::vec3 &end, float step_size, float max_diff);
	
	//Volume stuff
	WaterRegionType ReturnRegionType(const glm::vec3 &pos) const;
	bool InWater(const glm::vec3 &pos) const;
	bool InVWater(const glm::vec3 &pos) const;
	bool InLava(const glm::vec3 &pos) const;
	bool InLiquid(const glm::vec3 &pos) const;
	
private:
	struct impl;
	impl *imp;
};

#endif
