#include "water_portal.h"

//void WaterPortal::Connect(int id) {
//	m_connections.push_back(id);
//}
//
//void WaterPortal::Delete(int id) {
//	auto iter = m_connections.begin();
//	while(iter != m_connections.end()) {
//		if((*iter) == id) {
//			m_connections.erase(iter);
//			return;
//		}
//		++iter;
//	}
//}
//
//bool WaterPortal::HasConnection(int id) {
//	for(auto &conn_id : m_connections) {
//		if(conn_id == id) {
//			return true;
//		}
//	}
//	return false;
//}
//
//WaterPortalManager::WaterPortalManager(std::shared_ptr<EQPhysics> physics, const glm::vec3& min, const glm::vec3& max) {
//	m_physics = physics;
//	m_min = min;
//	m_max = max;
//	m_center = glm::vec3((min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f, (min.z + max.z) / 2.0f);
//	m_nodes_octree = std::unique_ptr<Octree<WaterPortal>>(new Octree<WaterPortal>(Octree<WaterPortal>::AABB(min, max)));
//}
//
//WaterPortalManager::~WaterPortalManager() {
//}
//
//void WaterPortalManager::GenerateNodes() {
//	float step_size = 5.0f;
//	int id = 1;
//	m_nodes.clear();
//
//	for(float x = m_min.x + (step_size / 2.0f); x < m_max.x; x += step_size)
//	{
//		for(float y = m_min.y + (step_size / 2.0f); y < m_max.y; y += step_size)
//		{
//			for(float z = m_min.z + (step_size / 2.0f); z < m_max.z; z += step_size)
//			{
//				glm::vec3 pos(x, y, z);
//				if(m_physics->InLiquid(pos) && !m_physics->IsUnderworld(pos)) {
//					WaterPortal *wp = new WaterPortal;
//					wp->id = id;
//					wp->m_pos = pos;
//					m_nodes_octree->Insert(pos, wp);
//					m_nodes[id] = std::unique_ptr<WaterPortal>(wp);
//					id++;
//				}
//			}
//		}
//	}
//
//	printf("Generated %u nodes\n", m_nodes.size());
//
//	for(auto &node : m_nodes) {
//		m_nodes_octree->TraverseRange(node.second->m_pos, 200.0f, [this, &node](const glm::vec3 &pos, WaterPortal *wp) 
//		{
//			if(node.second->id != wp->id && m_physics->CheckLOS(wp->m_pos, node.second->m_pos)) {
//				node.second->Connect(wp->id);
//				wp->Connect(node.second->id);
//			}
//		});
//	}
//
//	size_t num_conn = 0;
//	for(auto &node : m_nodes) {
//		num_conn += node.second->m_connections.size();
//	}
//}
//
//bool WaterPortalManager::SimplifyNodes() {
//	for(auto &node : m_nodes) {
//		if(CanCollapse(node.second.get())) {
//			Collapse(node.second.get());
//			printf("Collapsed node\n");
//			return true;
//		}
//	}
//
//	return false;
//}
//
//void WaterPortalManager::AppendModel(LineModel *mod) {
//	float model_size = 0.5f;
//	for(auto &iter : m_nodes) {
//		mod->AddBox(
//			glm::vec3(iter.second->m_pos.x - model_size,
//				iter.second->m_pos.y - model_size,
//				iter.second->m_pos.z - model_size),
//			glm::vec3(iter.second->m_pos.x + model_size,
//				iter.second->m_pos.y + model_size,
//				iter.second->m_pos.z + model_size));
//	}
//}
//
//bool WaterPortalManager::CanCollapse(WaterPortal *wp) {
//	//node A can be collapsed if all of it's connections point to each other
//
//	//for every child we have make sure their list contains every node in our list except their id
//	for(auto &child : wp->m_connections) {
//		auto child_node = m_nodes.find(child);
//
//		if(child_node == m_nodes.end()) {
//			continue;
//		}
//
//		for(auto &conn : wp->m_connections) {
//			if(conn != child_node->second->id && !child_node->second->HasConnection(conn)) {
//				return false;
//			}
//		}
//	}
//
//	return true;
//}
//
//void WaterPortalManager::Collapse(WaterPortal *wp) {
//	//make all your children nodes point to each other while deleting you
//	for(auto &child : wp->m_connections) {
//		auto child_node = m_nodes.find(child);
//
//		if(child_node == m_nodes.end()) {
//			continue;
//		}
//
//		child_node->second->Delete(wp->id);
//
//		for(auto &conn : wp->m_connections) {
//			if(conn != wp->id && !child_node->second->HasConnection(conn)) {
//				child_node->second->Connect(conn);
//			}
//		}
//	}
//
//	//remove yourself from nodes
//	m_nodes.erase(wp->id);
//
//	//remove yourself from octree
//	m_nodes_octree->Delete(wp);
//}