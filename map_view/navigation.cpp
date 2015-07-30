#include "navigation.h"
#include "imgui.h"
#include "oriented_bounding_box.h"
#define _USE_MATH_DEFINES
#include <math.h>

void Navigation::WorkerFunc() {
//	for(;;) {
//		m_nav_mesh_work_lock.lock();
//		if(m_nav_mesh_work.empty()) {
//			m_nav_mesh_work_lock.unlock();
//			return;
//		}
//
//		auto tile = m_nav_mesh_work.front();
//		m_nav_mesh_work.pop();
//		m_nav_mesh_work_lock.unlock();
//
//		ProcessTile(tile);
//		std::this_thread::sleep_for(std::chrono::milliseconds(10));
//	}
};

void Navigation::ProcessTile(const NavTile &tile) {
//	const float* verts = (float*)z_model->GetPositions().data();
//	const int nverts = z_model->GetPositions().size();
//	const int ntris = z_model->GetIndicies().size() / 3;
//
//	rcConfig config;
//	memset(&config, 0, sizeof(config));
//
//	//config.cs = m_cellSize;
//	//config.ch = m_cellHeight;
//	//config.walkableSlopeAngle = m_agentMaxSlope;
//	//config.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
//	//config.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
//	//config.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
//	//config.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
//	//config.maxSimplificationError = m_edgeMaxError;
//	//config.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
//	//config.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
//	//config.maxVertsPerPoly = (int)m_vertsPerPoly;
//	//config.tileSize = (int)m_tileSize;
//	//config.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
//	//config.width = m_cfg.tileSize + m_cfg.borderSize * 2;
//	//config.height = m_cfg.tileSize + m_cfg.borderSize * 2;
//	//config.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
//	//config.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
//
//	//rcVcopy(config.bmin, bmin);
//	//rcVcopy(config.bmax, bmax);
//	//config.bmin[0] -= config.borderSize*m_cfg.cs;
//	//config.bmin[2] -= config.borderSize*m_cfg.cs;
//	//config.bmax[0] += config.borderSize*m_cfg.cs;
//	//config.bmax[2] += config.borderSize*m_cfg.cs;
//
//	rcHeightfield *solid = rcAllocHeightfield();
//	if(!solid) {
//		return;
//	}
//
//	if(!rcCreateHeightfield(nullptr, *solid, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch))
//	{
//		rcFreeHeightField(solid);
//		return;
//	}
//
//	std::unique_ptr<unsigned char> triareas = std::unique_ptr<unsigned char>(new unsigned char[m_nav_mesh_tree->maxTrisPerChunk]);
//	if(!triareas) {
//		rcFreeHeightField(solid);
//		return;
//	}
//
//	float tbmin[2], tbmax[2];
//	tbmin[0] = config.bmin[0];
//	tbmin[1] = config.bmin[2];
//	tbmax[0] = config.bmax[0];
//	tbmax[1] = config.bmax[2];
//
//	int cid[512];
//	const int ncid = rcGetChunksOverlappingRect(m_nav_mesh_tree.get(), tbmin, tbmax, cid, 512);
//
//	if(!ncid) {
//		rcFreeHeightField(solid);
//		return;
//	}
//
//	int tile_tri_count = 0;
//	for(int i = 0; i < ncid; ++i)
//	{
//		const rcChunkyTriMeshNode& node = m_nav_mesh_tree->nodes[cid[i]];
//		const int* ctris = &m_nav_mesh_tree->tris[node.i * 3];
//		const int nctris = node.n;
//
//		tile_tri_count += nctris;
//
//		memset(triareas.get(), 0, nctris * sizeof(unsigned char));
//	}
}

void Navigation::BuildNavMesh() {
	if(!m_nav_mesh_tree || !z_model) {
		eqLogMessage(LogInfo, "Unable to create a nav mesh without a zone mesh or zone model.");
		return;
	}

	m_nav_mesh_work_lock.lock();
	bool work_pending = !m_nav_mesh_work.empty();
	m_nav_mesh_work_lock.unlock();

	if(work_pending) {
		eqLogMessage(LogInfo, "Pending nav mesh work, wont attempt to rebuild until there is no more pending work...");
		return;
	}

	int gw = 0, gh = 0;

	rcCalcGridSize(&z_model->GetAABBMin()[0], &z_model->GetAABBMax()[0], m_cell_size, &gw, &gh);

	eqLogMessage(LogInfo, "Grid size for cell size %.2f: (%d, %d)", m_cell_size, gw, gh);

	const int ts = (int)m_tile_size;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tile_size * m_cell_size;

	float m_tile_b_min[3];
	float m_tile_b_max[3];

	for(int y = 0; y < th; ++y)
	{
		for(int x = 0; x < tw; ++x)
		{
			m_tile_b_min[0] = z_model->GetAABBMin()[0] + x * tcs;
			m_tile_b_min[1] = z_model->GetAABBMin()[1];
			m_tile_b_min[2] = z_model->GetAABBMin()[2] + y * tcs;

			m_tile_b_max[0] = z_model->GetAABBMin()[0] + (x + 1)*tcs;
			m_tile_b_max[1] = z_model->GetAABBMax()[1];
			m_tile_b_max[2] = z_model->GetAABBMin()[2] + (y + 1)*tcs;

			eqLogMessage(LogInfo, "Queue tile (%d, %d), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)", x, y, 
						 m_tile_b_min[0], m_tile_b_min[1], m_tile_b_min[2],
						 m_tile_b_max[0], m_tile_b_max[1], m_tile_b_max[2]);

			NavTile tile;
			tile.x = x;
			tile.y = y;
			tile.min_x = m_tile_b_min[0];
			tile.min_y = m_tile_b_min[1];
			tile.min_z = m_tile_b_min[2];
			tile.max_x = m_tile_b_max[0];
			tile.max_y = m_tile_b_max[1];
			tile.max_z = m_tile_b_max[2];

			m_nav_mesh_work.push(tile);
		}
	}

	for(int i = 0; i < 4; ++i) {
		std::thread t1(&Navigation::WorkerFunc, this);
		t1.detach();
	}

}

void Navigation::ProcessNavMesh() {
	//std::function<void(NavTile nt)> worker_func;
	//NavTile tile;
	//bool launch_worker = false;
	//
	//m_nav_mesh_work_lock.lock();
	//if(!m_nav_mesh_work.empty() && m_nav_mesh_workers < MAX_NAV_THREADS) {
	//	tile = m_nav_mesh_work.front();
	//	m_nav_mesh_work.pop();
	//
	//	worker_func = [this](NavTile nt) {
	//		eqLogMessage(LogInfo, "Process tile (%d, %d), (%.2f, %.2f, %.2f), (%.2f, %.2f, %.2f)", nt.x, nt.y,
	//					 nt.min_x, nt.min_y, nt.min_z,
	//					 nt.max_x, nt.max_y, nt.max_z);
	//
	//		m_nav_mesh_work_lock.lock();
	//		m_nav_mesh_workers--;
	//		m_nav_mesh_work_lock.unlock();
	//	};
	//
	//	launch_worker = true;
	//}
	//
	////get work done
	//
	//m_nav_mesh_work_lock.unlock();
	//
	//if(launch_worker) {
	//	std::thread worker_thread(worker_func, tile);
	//	worker_thread.detach();
	//}
	//
	////process done work
}

//void Navigation::BuildNodeModel() {
//	m_nodes_model.reset(new Model());
//
//	auto &positions = m_nodes_model->GetPositions();
//	auto &indicies = m_nodes_model->GetIndicies();
//
//	m_nodes->TraverseSelection(m_bounds, [this, &positions, &indicies](const glm::vec3 &pos, Node *ent) {
//		float extent = 1.0f;
//		float scale = 1.0f;
//
//		glm::vec4 v1(-extent, extent, -extent, 1.0f);
//		glm::vec4 v2(-extent, extent, extent, 1.0f);
//		glm::vec4 v3(extent, extent, extent, 1.0f);
//		glm::vec4 v4(extent, extent, -extent, 1.0f);
//		glm::vec4 v5(-extent, -extent, -extent, 1.0f);
//		glm::vec4 v6(-extent, -extent, extent, 1.0f);
//		glm::vec4 v7(extent, -extent, extent, 1.0f);
//		glm::vec4 v8(extent, -extent, -extent, 1.0f);
//
//		glm::mat4 transformation = CreateRotateMatrix(0.0f, 0.0f, 0.0f);
//		transformation = CreateScaleMatrix(scale, scale, scale) * transformation;
//		transformation = CreateTranslateMatrix(pos.y, pos.x, pos.z) * transformation;
//
//		v1 = transformation * v1;
//		v2 = transformation * v2;
//		v3 = transformation * v3;
//		v4 = transformation * v4;
//		v5 = transformation * v5;
//		v6 = transformation * v6;
//		v7 = transformation * v7;
//		v8 = transformation * v8;
//
//		uint32_t current_index = (uint32_t)positions.size();
//		positions.push_back(glm::vec3(v1.y, v1.x, v1.z));
//		positions.push_back(glm::vec3(v2.y, v2.x, v2.z));
//		positions.push_back(glm::vec3(v3.y, v3.x, v3.z));
//		positions.push_back(glm::vec3(v4.y, v4.x, v4.z));
//		positions.push_back(glm::vec3(v5.y, v5.x, v5.z));
//		positions.push_back(glm::vec3(v6.y, v6.x, v6.z));
//		positions.push_back(glm::vec3(v7.y, v7.x, v7.z));
//		positions.push_back(glm::vec3(v8.y, v8.x, v8.z));
//
//		//top
//		indicies.push_back(current_index + 0);
//		indicies.push_back(current_index + 1);
//		indicies.push_back(current_index + 2);
//		indicies.push_back(current_index + 2);
//		indicies.push_back(current_index + 3);
//		indicies.push_back(current_index + 0);
//
//		//back
//		indicies.push_back(current_index + 1);
//		indicies.push_back(current_index + 2);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 5);
//		indicies.push_back(current_index + 1);
//
//		//bottom
//		indicies.push_back(current_index + 4);
//		indicies.push_back(current_index + 5);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 7);
//		indicies.push_back(current_index + 4);
//
//		//front
//		indicies.push_back(current_index + 0);
//		indicies.push_back(current_index + 3);
//		indicies.push_back(current_index + 7);
//		indicies.push_back(current_index + 7);
//		indicies.push_back(current_index + 4);
//		indicies.push_back(current_index + 0);
//
//		//left
//		indicies.push_back(current_index + 0);
//		indicies.push_back(current_index + 1);
//		indicies.push_back(current_index + 5);
//		indicies.push_back(current_index + 5);
//		indicies.push_back(current_index + 4);
//		indicies.push_back(current_index + 0);
//
//		//right
//		indicies.push_back(current_index + 3);
//		indicies.push_back(current_index + 2);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 6);
//		indicies.push_back(current_index + 7);
//		indicies.push_back(current_index + 3);
//	});
//
//	m_nodes_model->Compile();
//}
//
//Node* Navigation::AddNode(float x, float y, float z) {
//	ZoneMap::Vertex loc(x, y, z);
//	float best_z = z_map->FindBestZ(loc, nullptr);
//	if(best_z == BEST_Z_INVALID) {
//		return nullptr;
//	}
//
//	Node *n = new Node();
//	n->x = x;
//	n->y = y;
//	n->z = best_z;
//	n->id = m_node_id++;
//
//	for(int i = 0; i < 8; ++i) {
//		n->connected[i] = nullptr;
//	}
//
//	bool node_exists = false;
//	m_nodes->TraverseRange(glm::vec3(x, y, best_z), 2.5f, [&node_exists](const glm::vec3 &pos, Node *ent) {
//		node_exists = true;
//	});
//
//	if(node_exists) {
//		delete n;
//		return nullptr;
//	}
//
//	m_nodes->Insert(glm::vec3(x, y, best_z), n);
//	m_existing_nodes.push_back(std::unique_ptr<Node>(n));
//
//	return n;
//}
//
//void Navigation::Flood(float x, float y, float z) {
//	AddNode(x, y, z);
//
//	float step = 10.0f;
//	AddNode(x + step, y, z);
//	AddNode(x + step, y + step, z);
//	AddNode(x + step, y - step, z);
//	AddNode(x - step, y, z);
//	AddNode(x - step, y + step, z);
//	AddNode(x - step, y - step, z);
//	AddNode(x, y + step, z);
//	AddNode(x, y - step, z);
//}
