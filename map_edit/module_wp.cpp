#include "module_wp.h"

ModuleWP::ModuleWP()
{
	m_selected_node = -1;
	m_dirty = false;
	m_current_door_id = -1;
	m_current_teleport = 0;
	m_current_bidirectional = true;
}

ModuleWP::~ModuleWP()
{
}

void ModuleWP::OnLoad(Scene* s)
{
	m_scene = s;
	m_scene->RegisterHotkey(this, 500, GLFW_KEY_DELETE, false, false, false);
}

void ModuleWP::OnShutdown()
{
}

void ModuleWP::OnDrawMenu()
{
	if (ImGui::BeginMenu("WP"))
	{
		if (ImGui::MenuItem("Clear")) {
			
		}
		ImGui::EndMenu();
	}
}

void ModuleWP::OnDrawUI()
{
	ImGui::Begin("Waypoint Navigation");
	ImGui::End();

	if (m_selected_node != -1) {
		auto &node = m_nodes[m_selected_node];
		auto &io = ImGui::GetIO();
		bool update = false;
		if (io.KeysDown[GLFW_KEY_UP]) {
			update = true;
			node.y += 0.25f;
		}

		if (io.KeysDown[GLFW_KEY_DOWN]) {
			update = true;
			node.y -= 0.25f;
		}

		if (io.KeysDown[GLFW_KEY_RIGHT]) {
			update = true;
			node.x += 0.25f;
		}

		if (io.KeysDown[GLFW_KEY_LEFT]) {
			update = true;
			node.x -= 0.25f;
		}

		if (io.KeysDown[GLFW_KEY_PAGE_UP]) {
			update = true;
			node.z += 0.25f;
		}

		if (io.KeysDown[GLFW_KEY_PAGE_DOWN]) {
			update = true;
			node.z -= 0.25f;
		}

		if (update) {
			m_selected_renderable->SetLocation(glm::vec3(node.x, node.z, node.y));
			m_dirty = true;
			BuildVisualGraph();
		}
	}
}

void ModuleWP::OnDrawOptions()
{
}

void ModuleWP::OnSceneLoad(const char *zone_name)
{
	LoadPath();
}

void ModuleWP::OnSuspend()
{
	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleWP::OnResume()
{
	if (m_nodes_renderable) {
		m_scene->RegisterEntity(this, m_nodes_renderable.get(), true);
	}

	if (m_edges_renderable) {
		m_scene->RegisterEntity(this, m_edges_renderable.get());
	}

	if (m_selected_renderable) {
		m_scene->RegisterEntity(this, m_selected_renderable.get());
	}
}

bool ModuleWP::HasWork()
{
	return false;
}

bool ModuleWP::CanSave()
{
	return true;
}

void ModuleWP::Save()
{
	std::string filename = "maps/" + m_scene->GetZoneName() + ".path";
	FILE *f = fopen(filename.c_str(), "wb");
	if (f) {
		uint32_t version = 3;
		char magic[9] = { 'E', 'Q', 'E', 'M', 'U', 'P', 'A', 'T', 'H' };
		fwrite(magic, sizeof(magic), 1, f);
		fwrite(&version, sizeof(uint32_t), 1, f);

		uint32_t node_count = m_nodes.size();
		uint32_t edge_count = m_edges.size();

		fwrite(&node_count, sizeof(uint32_t), 1, f);
		fwrite(&edge_count, sizeof(uint32_t), 1, f);

		for (auto &node : m_nodes) {
			uint32_t id = node.id;
			float x = node.x;
			float y = node.y;
			float z = node.z;
			float best_z = node.best_z;

			fwrite(&id, sizeof(uint32_t), 1, f);
			fwrite(&x, sizeof(float), 1, f);
			fwrite(&y, sizeof(float), 1, f);
			fwrite(&z, sizeof(float), 1, f);
			fwrite(&best_z, sizeof(float), 1, f);
		}

		for (auto &edge : m_edges) {
			uint32_t from = edge.from;
			uint32_t to = edge.to;
			int8_t teleport = edge.teleport;
			float distance = edge.distance;
			int32_t door_id = edge.door_id;

			fwrite(&from, sizeof(uint32_t), 1, f);
			fwrite(&to, sizeof(uint32_t), 1, f);
			fwrite(&teleport, sizeof(int8_t), 1, f);
			fwrite(&distance, sizeof(float), 1, f);
			fwrite(&door_id, sizeof(int32_t), 1, f);
		}

		fclose(f);
	}
}

void ModuleWP::OnHotkey(int ident)
{
	if (ident == 500) {
		Delete();
	}
}

void ModuleWP::OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *non_collide_hit, const glm::vec3 *select_hit, Entity *selected)
{
	if (mouse_button != GLFW_MOUSE_BUTTON_1) {
		return;
	}

	auto &io = ImGui::GetIO();

	if (select_hit) {
		if (io.KeyShift) { //Connect node to node
			if (m_selected_node == -1) {
				return;
			}
			
			auto current = GetSelectedNode(glm::vec3(select_hit->x, select_hit->z, select_hit->y));
			if (current == -1) {
				return;
			}

			Connect(m_selected_node, current);
		}
		else if (io.KeyCtrl) { //Disconnect node to node
			if (m_selected_node == -1) {
				return;
			}

			auto current = GetSelectedNode(glm::vec3(select_hit->x, select_hit->z, select_hit->y));
			if (current == -1) {
				return;
			}

			Disconnect(m_selected_node, current);
		}
		else {
			auto current = GetSelectedNode(glm::vec3(select_hit->x, select_hit->z, select_hit->y));

			if (m_dirty) {
				m_selected_node = current;
				BuildVisualGraph(true);
				m_dirty = false;
			}
			else {
				m_selected_node = current;
				BuildVisualGraph();
			}
		}
	}
	else if(collide_hit) {
		if (io.KeyShift) { //Add new node
			Create(glm::vec3(collide_hit->x, collide_hit->z, collide_hit->y));
		}
		else {
			m_selected_node = -1;
			BuildVisualGraph();
		}
	}
}

void ModuleWP::LoadPath()
{
	m_selected_node = -1;
	m_nodes.clear();
	m_edges.clear();
	std::string filename = "maps/" + m_scene->GetZoneName() + ".path";
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		char magic[9] = { 0 };
		if (fread(magic, 9, 1, f) != 1) {
			fclose(f);
			return;
		}

		if (strncmp(magic, "EQEMUPATH", 9) != 0)
		{
			fclose(f);
			return;
		}

		uint32_t version = 0;
		if (fread(&version, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		uint32_t nodes = 0;
		if (fread(&nodes, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (version == 3) {
			LoadV3(f, nodes);
			return;
		} else if (version == 2) {
			LoadV2(f, nodes);
			return;
		}

		fclose(f);
	}
	else {
		BuildVisualGraph(true);
	}
}

void ModuleWP::LoadV2(FILE *f, uint32_t nodes)
{
	if (nodes == 0) {
		return;
	}

	m_nodes.resize(nodes);

	for (uint32_t i = 0; i < nodes; ++i) {
		uint16_t id = 0;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float best_z = 0.0f;

		if (fread(&id, sizeof(uint16_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&x, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&y, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&z, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&best_z, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		m_nodes[id].x = x;
		m_nodes[id].y = y;
		m_nodes[id].z = z;
		m_nodes[id].best_z = best_z;
		m_nodes[id].id = id;

		for (int i = 0; i < 50; ++i) {
			int16_t to = 0;
			float distance = 0.0f;
			uint8_t Teleport = 0;
			int16_t DoorID = 0;

			if (fread(&to, sizeof(int16_t), 1, f) != 1) {
				fclose(f);
				return;
			}

			if (fread(&distance, sizeof(float), 1, f) != 1) {
				fclose(f);
				return;
			}

			if (fread(&Teleport, sizeof(uint8_t), 1, f) != 1) {
				fclose(f);
				return;
			}

			if (fread(&DoorID, sizeof(int16_t), 1, f) != 1) {
				fclose(f);
				return;
			}

			if (to != -1) {
				WPEdge e;
				e.from = id;
				e.to = to;
				e.distance = distance;
				e.teleport = Teleport;
				e.door_id = DoorID;
				m_edges.push_back(e);
			}
		}
	}

	BuildVisualGraph();
	fclose(f);
}

void ModuleWP::LoadV3(FILE *f, uint32_t nodes)
{
	uint32_t edge_count = 0;
	if (fread(&edge_count, sizeof(uint32_t), 1, f) != 1) {
		fclose(f);
		return;
	}

	for (uint32_t i = 0; i < nodes; ++i) {
		uint32_t id = 0;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float best_z = 0.0f;

		if (fread(&id, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&x, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&y, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&z, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&best_z, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		WPNode n;
		n.x = x;
		n.y = y;
		n.z = z;
		n.best_z = best_z;
		n.id = id;
		m_nodes.push_back(n);
	}

	for (uint32_t j = 0; j < edge_count; ++j) {
		uint32_t from = 0;
		uint32_t to = 0;
		int8_t teleport = 0;
		float distance = 0.0f;
		int32_t door_id = 0;

		if (fread(&from, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&to, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&teleport, sizeof(uint8_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&distance, sizeof(float), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (fread(&door_id, sizeof(int32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		WPEdge e;
		e.distance = distance;
		e.door_id = door_id;
		e.teleport = teleport;
		e.from = from;
		e.to = to;
		m_edges.push_back(e);
	}

	BuildVisualGraph();
	fclose(f);
}

void ModuleWP::BuildVisualGraph(bool rebuild)
{
	bool update_mesh = false;
	if (!m_nodes_renderable) {
		m_nodes_renderable.reset(new DynamicGeometry());
		m_nodes_renderable->SetDoublePass(true);
		update_mesh = true;
	}
	else {
		m_nodes_renderable->Clear();
	}

	if (!m_edges_renderable) {
		m_edges_renderable.reset(new DynamicGeometry());
		m_edges_renderable->SetDrawType(GL_LINES);
		m_edges_renderable->SetLineWidth(2.0f);
		update_mesh = true;
	}
	else {
		m_edges_renderable->Clear();
	}

	if (m_selected_node == -1 && m_selected_renderable) {
		m_scene->UnregisterEntity(this, m_selected_renderable.get());
	}

	int32_t i = 0;
	for (auto &node : m_nodes) {
		float sz = 1.0f;

		if (rebuild) {
			m_nodes_renderable->AddBox(glm::vec3(node.x - sz, node.z - sz, node.y - sz), glm::vec3(node.x + sz, node.z + sz, node.y + sz), glm::vec3(0.0, 1.0, 0.0));
		}
		else {
			if (m_selected_node != i)
			{
				m_nodes_renderable->AddBox(glm::vec3(node.x - sz, node.z - sz, node.y - sz), glm::vec3(node.x + sz, node.z + sz, node.y + sz), glm::vec3(0.0, 1.0, 0.0));
			}
			else {
				if (!m_selected_renderable) {
					m_selected_renderable.reset(new DynamicGeometry());
					m_selected_renderable->SetDoublePass(true);
					m_selected_renderable->AddBox(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1), glm::vec3(1, 0, 0));
					m_selected_renderable->Update();
				}

				if (m_running) {
					m_selected_renderable->SetLocation(glm::vec3(node.x, node.z, node.y));
					m_scene->RegisterEntity(this, m_selected_renderable.get());
				}
			}
		}

		i++;
	}

	for (auto &edge : m_edges) {
		auto &from = m_nodes[edge.from];
		auto &to = m_nodes[edge.to];

		if (edge.from == m_selected_node || edge.to == m_selected_node) {
			m_edges_renderable->AddLine(glm::vec3(from.x, from.z, from.y), glm::vec3(to.x, to.z, to.y), glm::vec3(1.0, 1.0, 0.0));
		}
		else {
			m_edges_renderable->AddLine(glm::vec3(from.x, from.z, from.y), glm::vec3(to.x, to.z, to.y), glm::vec3(1.0, 1.0, 1.0));
		}
	}

	m_nodes_renderable->Update();
	m_edges_renderable->Update();

	if (update_mesh || rebuild) {
		if (m_running) {
			m_scene->RegisterEntity(this, m_edges_renderable.get(), false);
			m_scene->RegisterEntity(this, m_nodes_renderable.get(), true);
		}

		if (rebuild) {
			BuildVisualGraph();
		}
	}
}

int32_t ModuleWP::GetSelectedNode(const glm::vec3 &loc)
{
	float distance = FLT_MAX;
	int32_t current = -1;
	for (auto i = 0; i < m_nodes.size(); ++i) {
		auto &node = m_nodes[i];
		float d = Distance(glm::vec3(node.x, node.y, node.z), loc);
		if (d < distance) {
			current = i;
			distance = d;
		}
	}

	return current;
}

void ModuleWP::Connect(int selected, int current)
{
	if (m_current_bidirectional) {
		ConnectNodeToNode(selected, current);
		ConnectNodeToNode(current, selected);
		m_selected_node = current;
		BuildVisualGraph(true);
		return;
	}

	ConnectNodeToNode(selected, current);
	m_selected_node = current;
	BuildVisualGraph(true);
}

void ModuleWP::ConnectNodeToNode(int a, int b)
{
	bool connected = false;
	for (auto &e : m_edges) {
		if (e.from == a && e.to == b) {
			connected = true;
			break;
		}
	}

	if (!connected) {
		WPNode &n_a = m_nodes[a];
		WPNode &n_b = m_nodes[b];

		WPEdge e;
		e.door_id = m_current_door_id;
		e.teleport = m_current_teleport;
		e.distance = Distance(glm::vec3(n_a.x, n_a.y, n_a.z), glm::vec3(n_b.x, n_b.y, n_b.z));
		e.from = a;
		e.to = b;

		m_edges.push_back(e);
	}
}

void ModuleWP::Disconnect(int selected, int current)
{
	if (m_current_bidirectional) {
		DisconnectNodeToNode(selected, current);
		DisconnectNodeToNode(current, selected);
		BuildVisualGraph();
		return;
	}

	DisconnectNodeToNode(selected, current);
	BuildVisualGraph();
}

void ModuleWP::DisconnectNodeToNode(int a, int b)
{
	auto iter = m_edges.begin();
	while (iter != m_edges.end()) {
		if (iter->from == a && iter->to == b) {
			m_edges.erase(iter);
			return;
		}

		++iter;
	}
}

void ModuleWP::Create(const glm::vec3 &loc)
{
	WPNode n;
	n.id = m_nodes.size();
	n.x = loc.x;
	n.y = loc.y;
	n.z = loc.z + 3.0f;
	n.best_z = n.z;

	m_nodes.push_back(n);

	if (m_selected_node >= 0) {
		if (m_current_bidirectional) {
			ConnectNodeToNode(m_selected_node, n.id);
			ConnectNodeToNode(n.id, m_selected_node);
		}
		else {
			ConnectNodeToNode(m_selected_node, n.id);
		}
	}

	m_selected_node = n.id;
	BuildVisualGraph(true);
}

void ModuleWP::Delete()
{
	if (m_selected_node == -1) {
		return;
	}

	//Delete node
	//Update nodes where id > m_selected_node
	auto node = m_nodes.begin();
	while (node != m_nodes.end()) {
		if (node->id == m_selected_node) {
			node = m_nodes.erase(node);
			continue;
		}

		if (node->id > m_selected_node) {
			auto &n = *node;
			n.id--;
		}

		++node;
	}

	//Delete edges
	//Update edges where id > m_selected_node
	auto edge = m_edges.begin();
	while (edge != m_edges.end()) {
		if (edge->from == m_selected_node || edge->to == m_selected_node) {
			edge = m_edges.erase(edge);
			continue;
		}

		if (edge->from > m_selected_node) {
			auto &e = *edge;
			e.from--;
		}

		if (edge->to > m_selected_node) {
			auto &e = *edge;
			e.to--;
		}

		++edge;
	}

	m_selected_node = -1;
	BuildVisualGraph(true);
}
