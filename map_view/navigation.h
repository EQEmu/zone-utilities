#ifndef EQEMU_MAP_NAVIGATION_H
#define EQEMU_MAP_NAVIGATION_H

#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include "model.h"
#include "zone_map.h"
#include "water_map.h"
#include "log_macros.h"
#include "octree.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "recast_chunky_mesh.h"

struct NavTile
{
	int x;
	int y;
	float min_x;
	float min_y;
	float min_z;
	float max_x;
	float max_y;
	float max_z;
};

class Navigation
{
public:
	Navigation(ZoneMap *z_map, WaterMap *w_map, Model *z_model) {
		this->z_map = z_map; 
		this->w_map = w_map;
		this->z_model = z_model;
		m_cell_size = 0.3;
		m_tile_size = 48.0f;
		m_nav_mesh = dtAllocNavMesh();
		
		if(z_model) {
			m_nav_mesh_tree = std::unique_ptr<rcChunkyTriMesh>(new rcChunkyTriMesh);
			if(!rcCreateChunkyTriMesh((float*)z_model->GetPositions().data(), (int*)z_model->GetIndicies().data(), 
				(int)z_model->GetIndicies().size() / 3, 256, m_nav_mesh_tree.get())) {
				m_nav_mesh_tree.release();
			}
		}
	}
	~Navigation() { dtFreeNavMesh(m_nav_mesh); }

	Model *GetNavigationModel() { return m_nav_model.get(); }
	
	float &GetCellSize() { return m_cell_size; }
	float &GetTileSize() { return m_tile_size; }

	void BuildNavMesh();
	void ProcessNavMesh();
private:
	void WorkerFunc();
	void ProcessTile(const NavTile &tile);

	ZoneMap *z_map;
	WaterMap *w_map;
	Model *z_model;

	float m_cell_size;
	float m_tile_size;

	std::unique_ptr<Model> m_nav_model;
	dtNavMesh *m_nav_mesh;
	std::unique_ptr<rcChunkyTriMesh> m_nav_mesh_tree;
	std::mutex m_nav_mesh_work_lock;
	std::queue<NavTile> m_nav_mesh_work;
};

#endif
