#ifndef EQEMU_MAP_NAVIGATION_H
#define EQEMU_MAP_NAVIGATION_H

#include "model.h"
#include "zone_map.h"
#include "water_map.h"
#include <memory>
#include <list>

#define MAX_PATH_CONNECTIONS 8
class PathNode
{
public:
	PathNode(float x, float y, float z);
	~PathNode() { }

	void Connect(PathNode *to);

	float GetX() const { return x; }
	float GetY() const { return y; }
	float GetZ() const { return z; }
private:
	float x;
	float y;
	float z;
	PathNode* connections[MAX_PATH_CONNECTIONS];
};

class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map) { this->z_map = z_map; this->w_map = w_map; }
	~Navigation() { }

	void FillNodes(float x, float y, float z);
	void CompileModel();
	void DrawGUI(float x, float y, float z);

	Model* GetNodesModel() { return nodes_model; }
private:
	std::vector<std::unique_ptr<PathNode>> nodes;
	Model *nodes_model;
	ZoneMap *z_map;
	WaterMap *w_map;
};

#endif
