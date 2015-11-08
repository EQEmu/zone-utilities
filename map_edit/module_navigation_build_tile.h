#ifndef EQEMU_MAP_VIEW_MODULE_NAVIGATION_BUILD_TILE_H
#define EQEMU_MAP_VIEW_MODULE_NAVIGATION_BUILD_TILE_H

#include "module_navigation.h"

class ModuleNavigationBuildTile : public ThreadPoolWork
{
public:
	ModuleNavigationBuildTile(ModuleNavigation *nav_module, int x, int y, glm::vec3 tile_min, glm::vec3 tile_max, std::shared_ptr<EQPhysics> physics) {
		m_nav_module = nav_module;
		m_x = x;
		m_y = y;
		m_tile_min = tile_min;
		m_tile_max = tile_max;
		m_ctx.reset(new rcContext());

		m_nav_data = nullptr;
		m_nav_data_size = 0;

		m_physics = physics;
	}
	~ModuleNavigationBuildTile() { if(m_nav_data) { dtFree(m_nav_data); m_nav_data = nullptr; } }

	virtual void Run();
	virtual void Finished();

private:
	ModuleNavigation *m_nav_module;
	std::unique_ptr<rcContext> m_ctx;
	int m_x;
	int m_y; 
	glm::vec3 m_tile_min; 
	glm::vec3 m_tile_max;

	unsigned char* m_nav_data;
	int m_nav_data_size;

	std::shared_ptr<EQPhysics> m_physics;
};

#endif
