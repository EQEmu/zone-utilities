#ifndef EQEMU_MAP_NAVIGATION_H
#define EQEMU_MAP_NAVIGATION_H

#include <memory>
#include <thread>
#include <mutex>
#include <list>
#include <vector>
#include "model.h"
#include "shader.h"
#include "zone_map.h"
#include "water_map.h"
#include "log_macros.h"
#include "octree.h"
#include "Recast.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DebugDraw.h"
#include "DetourDebugDraw.h"

struct PathNode
{
	PathNode() { 
		id = 0;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	int id;
	float x;
	float y;
	float z;
	std::list<PathNode*> connected_to;
};

enum NavWorkStatus
{
	NavWorkNone,
	NavWorkNeedsCompile,
	NavWorkNavMeshCreateHeightField,
	NavWorkNavMeshMarkTriangles,
	NavWorkNavMeshFilterObstacles,
	NavWorkNavMeshBuildCompactHightField,
	NavWorkNavMeshErodeWalkableAreas,
	NavWorkNavMeshPartition,
	NavWorkNavMeshBuildContours,
	NavWorkNavMeshBuildPolyMesh,
	NavWorkNavMeshBuildPolyMeshDetail,
	NavWorkNavMeshConvertToDetourNM

};

enum NavMeshPartitionType
{
	NAVMESH_PARTITION_WATERSHED,
	NAVMESH_PARTITION_MONOTONE,
	NAVMESH_PARTITION_LAYERS,
};

struct NavMeshData
{
	NavMeshData() {
		m_cellSize = 0.3f;
		m_cellHeight = 0.2f;
		m_agentHeight = 2.0f;
		m_agentRadius = 0.6f;
		m_agentMaxClimb = 0.9f;
		m_agentMaxSlope = 45.0f;
		m_regionMinSize = 8;
		m_regionMergeSize = 20;
		m_edgeMaxLen = 12.0f;
		m_edgeMaxError = 1.3f;
		m_vertsPerPoly = 6.0f;
		m_detailSampleDist = 6.0f;
		m_detailSampleMaxError = 1.0f;
		m_partitionType = NAVMESH_PARTITION_MONOTONE;
		m_navMesh = nullptr;
	}

	~NavMeshData() {
		dtFreeNavMesh(m_navMesh);
	}

	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
	int m_partitionType;
	dtNavMesh* m_navMesh;
};

class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, Model *z_model) {
		this->z_map = z_map; 
		this->w_map = w_map;
		this->z_model = z_model;
		m_step_size = 3.0f;
		m_cull_close = 2.0f;
		m_max_slope_on_land = 60.0f;
		m_max_slope_into_water = 90.0f;
		m_node_id = 0;
		m_work_status = NavWorkNone;
	}
	~Navigation() { }

	void CreateNavMesh(Model *m);
	void BuildNavigationModel();

	void RenderGUI();
	void Draw(ShaderUniform *tint, bool wire);
private:
	void BuildNodeModel();
	void BuildNavMeshModel();

	PathNode *AttemptToAddWaterNode(float x, float y, float z);

	void SetStatus(NavWorkStatus status);
	NavWorkStatus GetStatus();

	ZoneMap *z_map;
	WaterMap *w_map;
	Model *z_model;
	glm::vec3 m_loc;

	//nav nodes
	float m_step_size;
	float m_cull_close;
	float m_max_slope_on_land;
	float m_max_slope_into_water;
	std::unique_ptr<Model> m_nav_nodes_model;
	std::unique_ptr<Octree<PathNode>> m_node_octree;
	std::vector<std::unique_ptr<PathNode>> m_nodes;
	int m_node_id;

	//nav mesh
	std::unique_ptr<Model> m_nav_mesh_model_lines;
	std::unique_ptr<Model> m_nav_mesh_model_points;
	std::unique_ptr<Model> m_nav_mesh_model_tris;
	NavMeshData m_nav_mesh_data;

	//shared work
	NavWorkStatus m_work_status;
	std::mutex m_work_lock;
};

class NavigationDebugDraw : public duDebugDraw
{
public:
	Model *lines;
	Model *points;
	Model *tris;

	virtual void depthMask(bool state) { }
	virtual void texture(bool state) { }
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end() { mode = 0; verts_in_use = 0; }
private:
	void CreatePrimitive();

	int mode;
	glm::vec3 verts[4];
	int verts_in_use;
};

#endif
