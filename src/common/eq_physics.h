#ifndef EQEMU_COMMON_EQ_PHYSICS_H
#define EQEMU_COMMON_EQ_PHYSICS_H

#include <vector>

#include "oriented_bounding_box.h"
#include "water_map.h"

enum EQPhysicsFlags
{
	CollidableWorld = 1,
	NonCollidableWorld = 2,
	Selectable = 4,
	NotSelectable = 8,
};

class btCollisionObject;
class EQPhysics
{
public:
	EQPhysics();
	~EQPhysics();
	
	//manipulation
	void SetWaterMap(WaterMap *w);
	WaterMap *GetWaterMap();
	void RegisterMesh(const std::string &ident, const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds, const glm::vec3 &pos, EQPhysicsFlags flag);
	void UnregisterMesh(const std::string &ident);
	void MoveMesh(const std::string &ident, const glm::vec3 &pos);
	void Step();

	//collision stuff
	bool CheckLOS(const glm::vec3 &src, const glm::vec3 &dest, glm::vec3 *result) const;
	bool GetRaycastClosestHit(const glm::vec3 &src, const glm::vec3 &dest, glm::vec3 &hit, std::string *name, int flag = CollidableWorld) const;
	float FindBestFloor(const glm::vec3 &start, glm::vec3 *result, glm::vec3 *normal) const;
	bool IsUnderworld(const glm::vec3 &point) const;
	
	//Volume stuff
	WaterRegionType ReturnRegionType(const glm::vec3 &pos) const;
	bool InWater(const glm::vec3 &pos) const;
	bool InVWater(const glm::vec3 &pos) const;
	bool InLava(const glm::vec3 &pos) const;
	bool InLiquid(const glm::vec3 &pos) const;
	
private:
	void GetEntityHit(const btCollisionObject *obj, std::string &out_ident) const;

	struct impl;
	impl *imp;
};

#endif
