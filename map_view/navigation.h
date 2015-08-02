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
#include "raycast_mesh.h"

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
	NavWorkLandNodePass,
	NavWorkWaterNodePass
};


class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, Model *z_model) {
		this->z_map = z_map; 
		this->w_map = w_map;
		this->z_model = z_model;
		m_step_size = 12;
		m_step_size_water = 12;
		m_max_slope_on_land = 60.0f;
		m_node_id = 0;
		m_work_status = NavWorkNone;
		m_selection = nullptr;
		m_nodes_mesh = nullptr;
		BuildSelectionModel();
	}
	~Navigation() { if(m_nodes_mesh) m_nodes_mesh->release(); }

	void UpdateCameraLocation(const glm::vec3 &loc) { m_loc = loc; }

	void ClearNavigation();
	void CalculateGraph(const glm::vec3 &min, const glm::vec3 &max);
	void AddLandNodes(const glm::vec2 &at);
	void AddWaterNode(const glm::vec3 &at);
	void AttemptToAddNode(float x, float y, float z);

	void BuildNavigationModel();

	void RenderGUI();
	void Draw(ShaderUniform *tint, bool wire);
	
	void DrawSelection(ShaderUniform *mvp, ShaderUniform *tint, glm::mat4 &view, glm::mat4 &proj);
	void ClearSelection() { m_selection = nullptr; }
	void SetSelection(PathNode *e) { m_selection = e; }
	void RaySelection(int mouse_x, int mouse_y, glm::mat4 &view, glm::mat4 &proj);
private:
	void BuildNodeModel();
	void BuildSelectionModel();

	void SetStatus(NavWorkStatus status);
	NavWorkStatus GetStatus();

	ZoneMap *z_map;
	WaterMap *w_map;
	Model *z_model;
	glm::vec3 m_loc;

	//selection
	PathNode *m_selection;
	std::unique_ptr<Model> m_selection_model;

	//nav nodes
	int m_step_size;
	int m_step_size_water;
	float m_max_slope_on_land;
	float m_max_slope_in_water;
	std::unique_ptr<Model> m_nav_nodes_model;
	std::unique_ptr<Octree<PathNode>> m_node_octree;
	std::vector<std::unique_ptr<PathNode>> m_nodes;
	RaycastMesh *m_nodes_mesh;
	int m_node_id;

	//shared work
	NavWorkStatus m_work_status;
	std::mutex m_work_lock;
};

#endif
