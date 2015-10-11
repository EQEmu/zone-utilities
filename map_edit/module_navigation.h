#ifndef EQEMU_MAP_VIEW_MODULE_NAVIGATION_H
#define EQEMU_MAP_VIEW_MODULE_NAVIGATION_H

#include <vector>

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DebugDraw.h>
#include <DetourDebugDraw.h>

#include "module.h"
#include "scene.h"
#include "line_model.h"
#include "rc_chunky_tri_mesh.h"
#include "thread_pool.h"
#include "nav_mesh_model.h"

class ModuleNavigation;
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

class NavigationDebugDraw : public duDebugDraw
{
public:
	virtual void depthMask(bool state) { }
	virtual void texture(bool state) { }
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end() { mode = 0; verts_in_use = 0; }

	ModuleNavigation *nav_module;
private:
	void CreatePrimitive();
	int mode;
	glm::vec3 verts[4];
	int verts_in_use;
};

enum NavigationPartitionType
{
	NAVIGATION_PARTITION_WATERSHED = 0,
	NAVIGATION_PARTITION_MONOTONE,
	NAVIGATION_PARTITION_LAYERS,
};

enum NavigationPolyFlags
{
	NavigationPolyFlagWalk = 0x01,
	NavigationPolyFlagSwim = 0x02,
	NavigationPolyFlagDisabled = 0x10,
	NavigationPolyFlagAll = 0xFFFF
};

class ModuleNavigation : public Module, public SceneHotkeyListener
{
public:
	ModuleNavigation();
	virtual ~ModuleNavigation();

	virtual const char *GetName() { return "Navigation"; };
	virtual void OnLoad(Scene *s);
	virtual void OnShutdown();
	virtual void OnDrawMenu();
	virtual void OnDrawUI();
	virtual void OnSceneLoad(const char *zone_name);
	virtual void OnSuspend();
	virtual void OnResume();
	virtual bool HasWork();
	virtual bool CanSave();
	virtual void Save();
	virtual void OnHotkey(int ident);
private:
	friend class ModuleNavigationBuildTile;
	friend class NavigationDebugDraw;
	void BuildNavigationMesh();
	void BuildWaterPortals();
	void CreateNavMeshModel();
	void CreateWaterPortalModel();

	Scene *m_scene;
	std::shared_ptr<rcChunkyTriMesh> m_chunky_mesh;
	std::unique_ptr<NavMeshModel> m_nav_mesh_renderable;
	std::unique_ptr<LineModel> m_water_portal_renderable;

	float m_cell_size;
	float m_cell_height;
	float m_agent_height;
	float m_agent_radius;
	float m_agent_max_climb;
	float m_agent_max_slope;
	float m_region_min_size;
	float m_region_merge_size;
	float m_edge_max_len;
	float m_edge_max_error;
	float m_verts_per_poly;
	float m_detail_sample_dist;
	float m_detail_sample_max_error;
	int m_partition_type;
	int m_max_tiles;
	int m_max_polys_per_tile;
	float m_tile_size;

	bool m_render_nav_mesh;

	dtNavMesh *m_nav_mesh;

	//work stuff
	int m_tiles_building;
	ThreadPool m_thread_pool;
};

#endif
