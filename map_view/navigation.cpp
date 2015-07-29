#include "navigation.h"
#include "imgui.h"

PathNode::PathNode(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;

	for(int i = 0; i < MAX_PATH_CONNECTIONS; ++i) {
		connections[i] = nullptr;
	}
}

void PathNode::Connect(PathNode *to) {
	for(int i = 0; i < MAX_PATH_CONNECTIONS; ++i)
	{
		if(!connections[i]) {
			connections[i] = to;
			return;
		}
	}
}

void Navigation::FillNodes(float x, float y, float z) {
	nodes.clear();

	//PathNode *start_n = new PathNode(x, y, z_map ? z_map->FindBestZ(ZoneMap::Vertex(x, y, z), nullptr) : z);
	//nodes.push_back(std::unique_ptr<PathNode>(start_n));
	//
	//float step_size = 10.0f;

	//PathNode *c_nodes[4];
	//
	//c_nodes[0] = new PathNode(x + step_size, y, z_map ? z_map->FindBestZ(ZoneMap::Vertex(x, y, z + 10), nullptr) : z);
	//c_nodes[1] = new PathNode(x + step_size, y, z_map ? z_map->FindBestZ(ZoneMap::Vertex(x, y, z + 10), nullptr) : z);
	//c_nodes[2] = new PathNode(x, y + step_size, z_map ? z_map->FindBestZ(ZoneMap::Vertex(x, y, z + 10), nullptr) : z);
	//c_nodes[3] = new PathNode(x, y - step_size, z_map ? z_map->FindBestZ(ZoneMap::Vertex(x, y, z + 10), nullptr) : z);
	
	//check los
	//check slope
	//check hazards
}

void Navigation::CompileModel() {
}

void Navigation::DrawGUI(float x, float y, float z) {
	ImGui::Begin("Navigation");
		if(ImGui::Button("Create Path Map")) {
			FillNodes(x, y, z);
		}
	ImGui::End();
}