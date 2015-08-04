#include "navigation.h"
#include "imgui.h"
#include "oriented_bounding_box.h"
#include "zone.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <gtc/matrix_transform.hpp>

void PathNode::Connect(PathNode *to, bool teleport) {
	for(auto &conn : connections) {
		if(conn.connected_to == to) {
			return;
		}
	}

	float weight_x = (to->pos.x - pos.x);
	float weight_y = (to->pos.y - pos.y);
	float weight_z = (to->pos.z - pos.z);

	PathNodeConnection conn;
	conn.connected_to = to;
	conn.teleport = teleport;
	conn.weight = sqrtf((weight_x * weight_x) + (weight_y * weight_y) + (weight_z * weight_z));

	connections.push_back(conn);
}

void PathNode::Link(PathNode *to, bool teleport) {
	Connect(to, teleport);
	to->Connect(this, teleport);
}

void Navigation::CalculateGraph(const glm::vec3 &min, const glm::vec3 &max) {
	auto status = GetStatus();
	if(status != NavWorkNone) {
		return;
	}

	SetStatus(NavWorkLandNodePass);

	float x_max = floor(max.x);
	float y_max = floor(max.y);
	float z_max = floor(max.z);
	for(float x = ceil(min.x); x < x_max; x += m_step_size) {
		for(float z = ceil(min.z); z < z_max; z += m_step_size) {
			AddLandNodes(glm::vec2(x, z));
		}
	}

	SetStatus(NavWorkWaterNodePass);
	for(float x = ceil(min.x); x < x_max; x += m_step_size_water) {
		for(float z = ceil(min.z); z < z_max; z += m_step_size_water) {
			for(float y = ceil(min.y); y < y_max; y += m_step_size_water) {
				AddWaterNode(glm::vec3(x, z, y));
			}
		}
	}

	SetStatus(NavWorkConnectionPass);
	for(auto &node : m_nodes) {
		m_node_octree->TraverseRange(node->pos, m_connect_range_land > m_connect_range_water ? m_connect_range_land : m_connect_range_water, 
									 [&node, this](const glm::vec3& pos, PathNode *ent) {
			if(ent == node.get()) {
				return;
			}
	
			float dist = glm::length(pos - node->pos);
			if(node->type == PathNodeWater || ent->type == PathNodeWater) {
				//one is a water node.
				
				if(dist > m_connect_range_water) {
					return;
				}

				if(z_map->CheckLoS(glm::vec3(node->pos.x, node->pos.y + m_agent_height, node->pos.z), glm::vec3(pos.x, pos.y, pos.z))) {
					node->Link(ent, false);
				}
			} else {
				//both land nodes

				if(dist > m_connect_range_land) {
					return;
				}

				if(z_map->CheckLosNoHazards(glm::vec3(node->pos.x, node->pos.y + m_agent_height, node->pos.z), glm::vec3(pos.x, pos.y, pos.z), m_hazard_step_size, m_max_hazard_diff)) {
					node->Link(ent, false);
				}
			}
		});
	}

	//optimization pass
	SetStatus(NavWorkOptimizationPass);
	//basic optimization remove any nodes without *any* connections
	auto iter = m_nodes.begin();
	while(iter != m_nodes.end()) {
		if((*iter)->connections.size() == 0) {
			iter = m_nodes.erase(iter);
			continue;
		}
		++iter;
	}

	SetStatus(NavWorkNeedsCompile);
}

void Navigation::AddLandNodes(const glm::vec2 &at) {
	glm::vec3 start(at.x, -BEST_Z_INVALID, at.y);
	glm::vec3 end(at.x, BEST_Z_INVALID, at.y);
	glm::vec3 hit;
	glm::vec3 normal;
	glm::vec3 normal_calc(0.0f, 1.0f, 0.0f);
	float hit_distance;

	while(z_map->Raycast(start, end, &hit, &normal, &hit_distance)) {
		if(hit_distance < 3.0f) {
			start.y = hit.y - 1.0f;
			continue;
		}

		float angle = glm::dot(normal, normal_calc);
		angle = acosf(angle) * 180.0f / (float)M_PI;

		if(!w_map->InLiquid(at.x, at.y, ceil(hit.y))) {
			if(angle < m_max_slope_on_land) {
				AttemptToAddNode(at.x, ceil(hit.y), at.y, PathNodeLand);
			}
		}

		start.y = hit.y - 1.0f;
	}
}

void Navigation::AddWaterNode(const glm::vec3 &at)  {
	if(!w_map->InLiquid(at.x, at.y, at.z)) {
		return;
	}

	if(z_map->IsUnderworld(glm::vec3(at.x, at.z, at.y))) {
		return;
	}

	AttemptToAddNode(at.x, at.z, at.y, PathNodeWater);
}

void Navigation::AttemptToAddNode(float x, float y, float z, PathNodeType type) {
	PathNode *node = new PathNode;
	node->id = m_node_id++;
	node->pos = glm::vec3(x, y, z);
	node->type = type;

	m_node_octree->Insert(node->pos, node);
	m_nodes.push_back(std::unique_ptr<PathNode>(node));
}

void Navigation::BuildNavigationModel() {
	if(GetStatus() != NavWorkNeedsCompile) {
		return;
	}

	SetStatus(NavWorkNone);
	m_nav_nodes_model.reset(new Model());

	BuildNodeModel();

	m_nav_nodes_model->Compile();
}

void Navigation::ClearNavigation() {
	m_nodes.clear();
	m_nodes_mesh.release();
	m_selection = nullptr;

	m_node_octree.reset(new Octree<PathNode>(Octree<PathNode>::AABB(z_model->GetAABBMin(), z_model->GetAABBMax())));
	m_nav_nodes_model.release();
	m_node_id = 0;
}

void AddCubeToModel(std::vector<glm::vec3> &positions, std::vector<unsigned int> &indicies, float x, float y, float z, float scale) {
	float extent = 1.0f;

	glm::vec4 v1(-extent, extent, -extent, 1.0f);
	glm::vec4 v2(-extent, extent, extent, 1.0f);
	glm::vec4 v3(extent, extent, extent, 1.0f);
	glm::vec4 v4(extent, extent, -extent, 1.0f);
	glm::vec4 v5(-extent, -extent, -extent, 1.0f);
	glm::vec4 v6(-extent, -extent, extent, 1.0f);
	glm::vec4 v7(extent, -extent, extent, 1.0f);
	glm::vec4 v8(extent, -extent, -extent, 1.0f);

	glm::mat4 transformation = CreateRotateMatrix(0.0f, 0.0f, 0.0f);
	transformation = CreateScaleMatrix(scale, scale, scale) * transformation;
	transformation = CreateTranslateMatrix(x, y, z) * transformation;

	v1 = transformation * v1;
	v2 = transformation * v2;
	v3 = transformation * v3;
	v4 = transformation * v4;
	v5 = transformation * v5;
	v6 = transformation * v6;
	v7 = transformation * v7;
	v8 = transformation * v8;

	uint32_t current_index = (uint32_t)positions.size();
	positions.push_back(glm::vec3(v1.y, v1.x, v1.z));
	positions.push_back(glm::vec3(v2.y, v2.x, v2.z));
	positions.push_back(glm::vec3(v3.y, v3.x, v3.z));
	positions.push_back(glm::vec3(v4.y, v4.x, v4.z));
	positions.push_back(glm::vec3(v5.y, v5.x, v5.z));
	positions.push_back(glm::vec3(v6.y, v6.x, v6.z));
	positions.push_back(glm::vec3(v7.y, v7.x, v7.z));
	positions.push_back(glm::vec3(v8.y, v8.x, v8.z));

	//top
	indicies.push_back(current_index + 0);
	indicies.push_back(current_index + 1);
	indicies.push_back(current_index + 2);
	indicies.push_back(current_index + 2);
	indicies.push_back(current_index + 3);
	indicies.push_back(current_index + 0);

	//back
	indicies.push_back(current_index + 1);
	indicies.push_back(current_index + 2);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 5);
	indicies.push_back(current_index + 1);

	//bottom
	indicies.push_back(current_index + 4);
	indicies.push_back(current_index + 5);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 7);
	indicies.push_back(current_index + 4);

	//front
	indicies.push_back(current_index + 0);
	indicies.push_back(current_index + 3);
	indicies.push_back(current_index + 7);
	indicies.push_back(current_index + 7);
	indicies.push_back(current_index + 4);
	indicies.push_back(current_index + 0);

	//left
	indicies.push_back(current_index + 0);
	indicies.push_back(current_index + 1);
	indicies.push_back(current_index + 5);
	indicies.push_back(current_index + 5);
	indicies.push_back(current_index + 4);
	indicies.push_back(current_index + 0);

	//right
	indicies.push_back(current_index + 3);
	indicies.push_back(current_index + 2);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 6);
	indicies.push_back(current_index + 7);
	indicies.push_back(current_index + 3);
}

void Navigation::BuildNodeModel() {
	m_nodes_mesh.release();
	auto &positions = m_nav_nodes_model->GetPositions();
	auto &indicies = m_nav_nodes_model->GetIndicies();

	for(auto &ent : m_nodes) {
		AddCubeToModel(positions, indicies, ent->pos.y, ent->pos.x, ent->pos.z, 0.6f);
	}

	m_nodes_mesh.reset(createRaycastMesh((RmUint32)positions.size(), (const RmReal*)&positions[0], (RmUint32)(indicies.size() / 3), &indicies[0]));
}

void Navigation::BuildSelectionModel() {
	m_selection_model.reset(new Model());

	auto &positions = m_selection_model->GetPositions();
	auto &indicies = m_selection_model->GetIndicies();
	AddCubeToModel(positions, indicies, 0.0f, 0.0f, 0.0f, 0.6f);

	m_selection_model->Compile();
}

void Navigation::SetStatus(NavWorkStatus status) {
	m_work_lock.lock();
	m_work_status = status;
	m_work_lock.unlock();
}

NavWorkStatus Navigation::GetStatus() {
	NavWorkStatus ret;
	m_work_lock.lock();
	ret = m_work_status;
	m_work_lock.unlock();
	return ret;
}

void Navigation::Draw() {
	if(!m_nav_nodes_model) {
		return;
	}

	bool wire = false;
	GLint params[2];
	glGetIntegerv(GL_POLYGON_MODE, params);
	if(params[0] == GL_LINE) {
		wire = true;
	}

	ShaderProgram shader = ShaderProgram::Current();
	ShaderUniform tint = shader.GetUniformLocation("Tint");
	
	if(!wire) {
		glm::vec4 tnt(0.5f, 1.0f, 0.7f, 1.0f);
		tint.SetValuePtr4(1, &tnt[0]);
	}

	glDisable(GL_CULL_FACE);
	m_nav_nodes_model->Draw();
	glEnable(GL_CULL_FACE);
}

void Navigation::DrawSelection() {
	if(!m_selection_model || !m_selection) {
		return;
	}

	ShaderProgram shader = ShaderProgram::Current();
	ShaderUniform mvp = shader.GetUniformLocation("MVP");
	ShaderUniform tint = shader.GetUniformLocation("Tint");

	glm::vec4 tnt(1.0f, 0.5f, 0.0f, 1.0f);
	tint.SetValuePtr4(1, &tnt[0]);
	
	glm::mat4 model = glm::mat4(1.0);
	model[3][0] = m_selection->pos.x;
	model[3][1] = m_selection->pos.y;
	model[3][2] = m_selection->pos.z;
	
	glm::mat4 mvp_mat = m_camera.GetProjMat() * m_camera.GetViewMat() * model;
	mvp.SetValueMatrix4(1, false, &mvp_mat[0][0]);
	
	glDisable(GL_CULL_FACE);
	m_selection_model->Draw();
	glEnable(GL_CULL_FACE);
}

void Navigation::DrawSelectionConnections() {
	if(!m_selection_connection_model || !m_selection) {
		return;
	}

	ShaderProgram shader = ShaderProgram::Current();
	ShaderUniform tint = shader.GetUniformLocation("Tint");

	glm::vec4 tnt(0.0f, 0.5f, 1.0f, 1.0f);
	tint.SetValuePtr4(1, &tnt[0]);

	glLineWidth(3.0f);
	glDisable(GL_CULL_FACE);
	m_selection_connection_model->Draw(GL_LINES);
	glEnable(GL_CULL_FACE);
	glLineWidth(1.0f);
}

void Navigation::RenderGUI() {
	NavWorkStatus status = GetStatus();

	if(status == NavWorkNeedsCompile) {
		BuildNavigationModel();
	}

	ImGui::Begin("Navigation", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	if(m_selection) {
		ImGui::Text("Selected node: %i at (%.2f, %.2f %.2f)", m_selection->id, m_selection->pos.x, m_selection->pos.z, m_selection->pos.y);
		if(m_selection->type == PathNodeLand) {
			glm::vec3 adjusted = m_selection->pos;
			adjusted.y += m_agent_height;
			ImGui::Text("Could connect: %s", z_map->CheckLosNoHazards(m_camera.GetLoc(), adjusted, m_hazard_step_size, m_max_hazard_diff) ? "true" : "false");
		} else {
			ImGui::Text("Could connect: %s", z_map->CheckLoS(m_camera.GetLoc(), m_selection->pos) ? "true" : "false");
		}
	} else {
		ImGui::Text("Selected node:");
		ImGui::Text("Could connect:");
	}

	ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Once);
	if(ImGui::TreeNode("Nodes")) {
		switch(status) {
		case NavWorkLandNodePass:
			ImGui::Text("Laying down land nodes");
			break;
		case NavWorkWaterNodePass:
			ImGui::Text("Laying down water nodes");
			break;
		case NavWorkConnectionPass:
			ImGui::Text("Connecting nodes");
			break;
		case NavWorkOptimizationPass:
			ImGui::Text("Optimizing connections");
			break;
		default:
		{

			ImGui::SliderFloat("Max voxel angle on land", &m_max_slope_on_land, 0.0f, 360.0f);
			ImGui::SliderInt("Step size", &m_step_size, 1, 50);
			ImGui::SliderInt("Water step size", &m_step_size_water, 1, 100);

			ImGui::SliderFloat("Hazard detection step size", &m_hazard_step_size, 1.0f, 100.0f);
			ImGui::SliderFloat("Hazard detection max diff", &m_max_hazard_diff, 0.0f, 100.0f);
			ImGui::SliderFloat("Hazard detection agent height", &m_agent_height, 0.0f, 100.0f);

			ImGui::SliderFloat("Automatic connect range (land)", &m_connect_range_land, 0.0f, 250.0f);
			ImGui::SliderFloat("Automatic connect range (water)", &m_connect_range_water, 0.0f, 250.0f);

			if(ImGui::Button("Calculate Navigation")) {
				ClearNavigation();
				std::thread t(&Navigation::CalculateGraph, this, z_model->GetAABBMin(), z_model->GetAABBMax());
				//std::thread t(&Navigation::CalculateGraph, this, glm::vec3(-100.0f, -100.0f, -100.0f), glm::vec3(100.0f, 100.0f, 100.0f));
				t.detach();
			}
		}
		}

		if(ImGui::TreeNode("NavMesh")) {
			ImGui::Text("NavMesh stuff here...");
		}
	}
	ImGui::End();
}

void Navigation::RaySelection(int mouse_x, int mouse_y) {
	if(!m_node_octree) {
		return;
	}

	glm::vec3 start;
	glm::vec3 end;

	glm::vec4 start_ndc(((float)mouse_x / (float)RES_X - 0.5f) * 2.0f, ((float)mouse_y / (float)RES_Y - 0.5f) * 2.0f, -1.0, 1.0f);
	glm::vec4 end_ndc(((float)mouse_x / (float)RES_X - 0.5f) * 2.0f, ((float)mouse_y / (float)RES_Y - 0.5f) * 2.0f, 0.0, 1.0f);

	glm::mat4 inverse_proj = glm::inverse(m_camera.GetProjMat());
	glm::mat4 inverse_view = glm::inverse(m_camera.GetViewMat());

	glm::vec4 start_camera = inverse_proj * start_ndc;
	start_camera /= start_camera.w;

	glm::vec4 start_world = inverse_view * start_camera;
	start_world /= start_world.w;

	glm::vec4 end_camera = inverse_proj * end_ndc;
	end_camera /= end_camera.w;

	glm::vec4 end_world = inverse_view * end_camera;
	end_world /= end_world.w;

	glm::vec3 dir_world(end_world - start_world);
	dir_world = glm::normalize(dir_world);

	start = glm::vec3(start_world);
	end = glm::normalize(dir_world) * 100000.0f;

	glm::vec3 hit;
	glm::vec3 normal;
	float hit_dist;
	m_nodes_mesh->raycast((RmReal*)&start, (RmReal*)&end, (RmReal*)&hit, (RmReal*)&normal, (RmReal*)&hit_dist);

	m_node_octree->TraverseRange(hit, 1.1f, [this](const glm::vec3 &loc, PathNode *ent) {
		SetSelection(ent);
	});
}

void Navigation::SetSelection(PathNode *e) {
	m_selection = e;

	if(m_selection) {
		m_selection_connection_model.reset(new Model());

		unsigned int ind = 0;
		auto &positions = m_selection_connection_model->GetPositions();
		auto &inds = m_selection_connection_model->GetIndicies();
		for(auto &connection : m_selection->connections) {
			positions.push_back(m_selection->pos);
			positions.push_back(connection.connected_to->pos);

			inds.push_back(ind++);
			inds.push_back(ind++);
		}

		m_selection_connection_model->Compile();
	}
}
