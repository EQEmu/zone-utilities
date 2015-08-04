#ifndef EQEMU_MAP_NAVIGATION_H
#define EQEMU_MAP_NAVIGATION_H

#include <memory>
#include <thread>
#include <mutex>
#include <vector>
#include "camera.h"
#include "model.h"
#include "shader.h"
#include "zone_map.h"
#include "water_map.h"
#include "log_macros.h"
#include "octree.h"
#include "raycast_mesh.h"

enum PathNodeType
{
	PathNodeLand,
	PathNodeWater
};

class PathNode;
struct PathNodeConnection
{
	float weight;
	bool teleport;
	PathNode *connected_to;
};

class PathNode
{
public:
	PathNode() { 
		id = 0;
	}

	void Connect(PathNode *to, bool teleport); //one way connection
	void Link(PathNode *to, bool teleport); //two way connection

	int id;
	glm::vec3 pos;
	PathNodeType type;
	std::vector<PathNodeConnection> connections;
};

enum NavWorkStatus
{
	NavWorkNone,
	NavWorkNeedsCompile,
	NavWorkLandNodePass,
	NavWorkWaterNodePass,
	NavWorkConnectionPass,
	NavWorkOptimizationPass
};

class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, Model *z_model, Camera &cam) : m_camera(cam) {
		this->z_map = z_map; 
		this->w_map = w_map;
		this->z_model = z_model;
		m_step_size = 12;
		m_step_size_water = 12;
		m_max_slope_on_land = 60.0f;
		m_hazard_step_size = 3.0f;
		m_max_hazard_diff = 10.0f;
		m_agent_height = 1.0f;
		m_connect_range_land = 20.0f;
		m_connect_range_water = 20.0f;

		m_node_id = 0;
		m_work_status = NavWorkNone;
		m_selection = nullptr;
		BuildSelectionModel();
	}
	~Navigation() { }

	void ClearNavigation();
	void CalculateGraph(const glm::vec3 &min, const glm::vec3 &max);
	void AddLandNodes(const glm::vec2 &at);
	void AddWaterNode(const glm::vec3 &at);
	void AttemptToAddNode(float x, float y, float z, PathNodeType type);

	void BuildNavigationModel();

	void RenderGUI();
	void Draw();
	
	void DrawSelection();
	void DrawSelectionConnections();
	void SetSelection(PathNode *e);
	void RaySelection(int mouse_x, int mouse_yj);
private:
	void BuildNodeModel();
	void BuildSelectionModel();

	void SetStatus(NavWorkStatus status);
	NavWorkStatus GetStatus();

	Camera &m_camera;
	ZoneMap *z_map;
	WaterMap *w_map;
	Model *z_model;

	//selection
	PathNode *m_selection;
	std::unique_ptr<Model> m_selection_model;
	std::unique_ptr<Model> m_selection_connection_model;

	//nav nodes
	int m_step_size;
	int m_step_size_water;
	float m_max_slope_on_land;
	float m_max_slope_in_water;
	float m_connect_range_land;
	float m_connect_range_water;
	float m_hazard_step_size;
	float m_max_hazard_diff;
	float m_agent_height;
	std::unique_ptr<Model> m_nav_nodes_model;
	std::unique_ptr<Octree<PathNode>> m_node_octree;
	std::vector<std::unique_ptr<PathNode>> m_nodes;
	std::unique_ptr<RaycastMesh> m_nodes_mesh;
	int m_node_id;

	//shared work
	NavWorkStatus m_work_status;
	std::mutex m_work_lock;
};

#endif
