#ifndef EQEMU_MAP_NAVIGATION_H
#define EQEMU_MAP_NAVIGATION_H

#include "model.h"
#include "zone_map.h"
#include "water_map.h"
#include "octree.h"
#include <memory>
#include <vector>

struct Node
{
	float x;
	float y;
	float z;
	Node *connected[8];
};

class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, glm::vec3 min, glm::vec3 max) : m_bounds(min, max) {
		this->z_map = z_map; 
		this->w_map = w_map;
		m_nodes = std::unique_ptr<Octree<Node>>(new Octree<Node>(m_bounds));
	}
	~Navigation() { }

	Model *GetNodesModel() { return m_nodes_model.get(); }
	Model *GetConnectionModel() { return m_connection_model.get(); }

	void BuildNodeModel();
	void AddNode(float x, float y, float z);
private:
	ZoneMap *z_map;
	WaterMap *w_map;

	Octree<Node>::AABB m_bounds;
	std::unique_ptr<Model> m_nodes_model;
	std::unique_ptr<Model> m_connection_model;
	std::unique_ptr<Octree<Node>> m_nodes;
	std::vector<std::unique_ptr<Node>> m_existing_nodes;
};

#endif
