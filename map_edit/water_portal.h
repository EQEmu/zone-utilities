#ifndef EQEMU_MAP_VIEW_WATER_PORTAL_H
#define EQEMU_MAP_VIEW_WATER_PORTAL_H

#include <memory>
#include <vector>
#include <map>

#define GLM_FORCE_RADIANS
#include <glm.hpp>

#include "eq_physics.h"
#include "octree.h"

class LineModel;
class WaterPortalManager
{
public:
	WaterPortalManager(std::shared_ptr<EQPhysics> physics, const glm::vec3& min, const glm::vec3& max);
	~WaterPortalManager();

	void Subdivide();
	void AppendModel(LineModel *mod);
private:
	std::shared_ptr<EQPhysics> m_physics;
	glm::vec3 m_min;
	glm::vec3 m_max;
};

#endif
