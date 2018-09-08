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
#include "log/logger_interface.h"

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
	NavigationAreaFlagPortal,
	NavigationAreaFlagPrefer,
	NavigationAreaFlagDisabled,
};

enum NavigationPolyFlags
{
	NavigationPolyFlagNormal = 1,
	NavigationPolyFlagWater = 2,
	NavigationPolyFlagLava = 4,
	NavigationPolyFlagZoneLine = 8,
	NavigationPolyFlagPvP = 16,
	NavigationPolyFlagSlime = 32,
	NavigationPolyFlagIce = 64,
	NavigationPolyFlagVWater = 128,
	NavigationPolyFlagGeneralArea = 256,
	NavigationPolyFlagPortal = 512,
	NavigationPolyFlagPrefer = 1024,
	NavigationPolyFlagDisabled = 2048,
	NavigationPolyFlagAll = 0xFFFF,
	NavigationPolyFlagNotDisabled = NavigationPolyFlagAll ^ NavigationPolyFlagDisabled
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
	virtual void OnDrawOptions();
	virtual void OnSceneLoad(const char *zone_name);
	virtual void OnSuspend();
	virtual void OnResume();
	virtual bool HasWork();
	virtual bool CanSave();
	virtual void Save();
	virtual void OnHotkey(int ident);
	virtual void OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *select_hit, Entity *selected);
private:
	friend class ModuleNavigationBuildTile;
	friend class NavigationDebugDraw;
	void Clear();
	void DrawNavMeshGenerationUI();
	void DrawTestUI();
	void DrawMeshConnectionUI();
	void BuildNavigationMesh();
	void InitNavigationMesh();
	void BuildTile(const glm::vec3 &pos);
	void RemoveTile(const glm::vec3 &pos);
	void CreateChunkyTriMesh(std::shared_ptr<ZoneMap> zone_geo);
	void CreateNavMeshModel();
	void SetNavigationTestNodeStart(const glm::vec3 &p);
	void SetNavigationTestNodeEnd(const glm::vec3 &p);
	void CalcPath();
	void SaveNavSettings();
	bool LoadNavSettings();
	void SaveNavMesh();
	void LoadNavMesh();
	void LoadVolumes();
	void AddMeshConnection(const glm::vec3 &start, const glm::vec3 &end, float radius, unsigned char dir, unsigned char area, unsigned short flags);
	void DeleteMeshConnection(unsigned int i);
	void ClearConnections();
	void UpdateConnectionsModel();

	Scene *m_scene;
	std::shared_ptr<rcChunkyTriMesh> m_chunky_mesh;

	int m_mode;	

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

	//off-mesh-cons
	std::vector<glm::vec3> m_connection_verts;
	std::vector<float> m_connection_rads;
	std::vector<unsigned char> m_connection_dirs;
	std::vector<unsigned char> m_connection_areas;
	std::vector<unsigned short> m_connection_flags;
	std::vector<unsigned int> m_connection_ids;
	int m_connection_dir;
	int m_connection_area;
	float m_connection_radius;
	unsigned int m_connection_count;
	unsigned int m_connection_id_counter;

	glm::vec3 m_conn_start;
	bool m_conn_start_set;

	dtNavMesh *m_nav_mesh;
	std::unique_ptr<DebugDraw> m_nav_mesh_renderable;

	//connections
	std::unique_ptr<DebugDraw> m_connections_renderable;

	//path
	std::unique_ptr<DynamicGeometry> m_start_path_renderable;
	std::unique_ptr<DynamicGeometry> m_end_path_renderable;
	std::unique_ptr<DynamicGeometry> m_path_renderable;

	glm::vec3 m_path_start;
	bool m_path_start_set;

	glm::vec3 m_path_end;
	bool m_path_end_set;

	float m_path_costs[NavigationAreaFlagDisabled];

	std::vector<RegionVolume> m_volumes;

	int m_work_pending;

	std::shared_ptr<EQEmu::ILogger> _logger;
};

#endif
