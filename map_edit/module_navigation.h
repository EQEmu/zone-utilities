#ifndef EQEMU_MAP_VIEW_MODULE_NAVIGATION_H
#define EQEMU_MAP_VIEW_MODULE_NAVIGATION_H

#include <vector>

#include <Recast.h>
#include <DetourNavMesh.h>
#include <DebugDraw.h>
#include <DetourDebugDraw.h>

#include "module.h"
#include "scene.h"
#include "rc_chunky_tri_mesh.h"
#include "thread_pool.h"
#include "debug_draw.h"

enum NavigationPartitionType
{
	NAVIGATION_PARTITION_WATERSHED = 0,
	NAVIGATION_PARTITION_MONOTONE,
	NAVIGATION_PARTITION_LAYERS,
};

enum NavigationAreaFlags
{
	NavigationAreaFlagNormal,
	NavigationAreaFlagWater,
	NavigationAreaFlagLava,
	NavigationAreaFlagZoneLine,
	NavigationAreaFlagPvP,
	NavigationAreaFlagSlime,
	NavigationAreaFlagIce,
	NavigationAreaFlagVWater,
	NavigationAreaFlagGeneralArea,
	NavigationAreaFlagDisabled,
};

enum NavigationPolyFlags
{
	NavigationPolyFlagNormal = 0x01,
	NavigationPolyFlagWater = 0x02,
	NavigationPolyFlagLava = 0x04,
	NavigationPolyFlagZoneLine = 0x08,
	NavigationPolyFlagPvP = 0x10,
	NavigationPolyFlagSlime = 0x20,
	NavigationPolyFlagIce = 0x40,
	NavigationPolyFlagVWater = 0x80,
	NavigationPolyFlagGeneralArea = 0x100,
	NavigationPolyFlagDisabled = 0x200,
	NavigationPolyFlagAll = 0xFFFF
};

struct RegionVolume {
	float verts[4 * 3];
	float min;
	float max;
	NavigationAreaFlags area_type;
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

	DebugDraw *model;
private:
	void CreatePrimitive();
	int mode;
	glm::vec3 verts[4];
	glm::vec3 vert_colors[4];
	int verts_in_use;
};

class ModuleNavigation : public Module, public SceneHotkeyListener
{
public:
	enum Mode
	{
		ModeNone = 0,
		ModeNavMeshGen,
		ModeNavMeshConnections,
		ModeTestNavigation,
	};

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
	virtual void OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *non_collide_hit, const glm::vec3 *select_hit, Entity *selected);
private:
	friend class ModuleNavigationBuildTile;
	friend class NavigationDebugDraw;
	void Clear();
	void UpdateBoundingBox();
	void DrawNavMeshGenerationUI();
	void DrawTestUI();
	void BuildNavigationMesh();
	void CreateChunkyTriMesh(std::shared_ptr<ZoneMap> zone_geo);
	void CreateNavMeshModel();
	void SetNavigationTestNodeStart(const glm::vec3 &p);
	void SetNavigationTestNodeEnd(const glm::vec3 &p);
	void CalcPath();
	void InitVolumes();
	void SaveNavSettings();
	bool LoadNavSettings();
	void SaveNavMesh();

	Scene *m_scene;
	std::shared_ptr<rcChunkyTriMesh> m_chunky_mesh;

	int m_mode;

	//bounds
	std::unique_ptr<DynamicGeometry> m_bounding_box_renderable;
	glm::vec3 m_bounding_box_min;
	glm::vec3 m_bounding_box_max;

	//nav mesh generation vars
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

	dtNavMesh *m_nav_mesh;
	std::unique_ptr<DebugDraw> m_nav_mesh_renderable;

	//debug
	std::unique_ptr<DebugDraw> m_debug_renderable;

	//path
	std::unique_ptr<DynamicGeometry> m_start_path_renderable;
	std::unique_ptr<DynamicGeometry> m_end_path_renderable;
	std::unique_ptr<DynamicGeometry> m_path_renderable;

	glm::vec3 m_path_start;
	bool m_path_start_set;

	glm::vec3 m_path_end;
	bool m_path_end_set;

	float m_path_costs[NavigationAreaFlagDisabled];

	//volume
	std::vector<RegionVolume> m_volumes;

	int m_work_pending;
	ThreadPool m_thread_pool;
};

#endif
