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
	NavWorkHorizontalPass
};


class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, Model *z_model) {
		this->z_map = z_map; 
		this->w_map = w_map;
		this->z_model = z_model;
		m_step_size = 3;
		m_cull_close = 2.0f;
		m_max_slope_on_land = 60.0f;
		m_max_slope_in_water = 90.0f;
		m_node_id = 0;
		m_work_status = NavWorkNone;
	}
	~Navigation() { }

	void CalculateGraph(const glm::vec3 &min, const glm::vec3 &max);
	void CalculateGraphAt(const glm::vec2 &at);
	PathNode *AttemptToAddNode(float x, float y, float z);

	void BuildNavigationModel();

	void RenderGUI();
	void Draw(ShaderUniform *tint, bool wire);
private:
	void BuildNodeModel();

	void SetStatus(NavWorkStatus status);
	NavWorkStatus GetStatus();

	ZoneMap *z_map;
	WaterMap *w_map;
	Model *z_model;
	glm::vec3 m_loc;

	//nav nodes
	int m_step_size;
	float m_cull_close;
	float m_max_slope_on_land;
	float m_max_slope_in_water;
	std::unique_ptr<Model> m_nav_nodes_model;
	std::unique_ptr<Octree<PathNode>> m_node_octree;
	std::vector<std::unique_ptr<PathNode>> m_nodes;
	int m_node_id;

	//shared work
	NavWorkStatus m_work_status;
	std::mutex m_work_lock;
};

#endif
