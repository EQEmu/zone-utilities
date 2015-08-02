#include "navigation.h"
#include "imgui.h"
#include "oriented_bounding_box.h"
#define _USE_MATH_DEFINES
#include <math.h>

void Navigation::CalculateGraph(const glm::vec3 &min, const glm::vec3 &max) {
	auto status = GetStatus();
	if(status != NavWorkNone) {
		return;
	}

	SetStatus(NavWorkHorizontalPass);
	
	m_node_octree.reset(new Octree<PathNode>(Octree<PathNode>::AABB(z_model->GetAABBMin(), z_model->GetAABBMax())));
	m_nodes.clear();

	float x_max = floor(max.x);
	float z_max = floor(max.z);
	for(float x = ceil(min.x); x < x_max; x += m_step_size) {
		for(float z = ceil(min.z); z < z_max; z += m_step_size) {
			CalculateGraphAt(glm::vec2(x, z));
		}
	}

	SetStatus(NavWorkNeedsCompile);
}

void Navigation::CalculateGraphAt(const glm::vec2 &at) {
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
				AttemptToAddNode(at.x, ceil(hit.y), at.y);
			}
		}

		start.y = hit.y - 1.0f;
	}
	
}

PathNode *Navigation::AttemptToAddNode(float x, float y, float z) {
	PathNode *node = new PathNode;
	node->id = m_node_id++;
	node->x = x;
	node->y = y;
	node->z = z;
	
	m_node_octree->Insert(glm::vec3(x, y, z), node);
	m_nodes.push_back(std::unique_ptr<PathNode>(node));
	return node;
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

void Navigation::BuildNodeModel() {
	if(!z_model) {
		return;
	}

	auto &positions = m_nav_nodes_model->GetPositions();
	auto &indicies = m_nav_nodes_model->GetIndicies();

	for(auto &ent : m_nodes) {
		float extent = 1.0f;
		float scale = 0.4f;

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
		transformation = CreateTranslateMatrix(ent->y, ent->x, ent->z) * transformation;

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

void Navigation::Draw(ShaderUniform *tint, bool wire) {
	if(!m_nav_nodes_model) {
		return;
	}
	
	if(!wire) {
		glm::vec4 tnt(0.5f, 1.0f, 0.7f, 1.0f);
		tint->SetValuePtr4(1, &tnt[0]);
	}

	glDisable(GL_CULL_FACE);
	m_nav_nodes_model->Draw();
	glEnable(GL_CULL_FACE);
}

void Navigation::RenderGUI() {
	NavWorkStatus status = GetStatus();

	if(status == NavWorkNeedsCompile) {
		BuildNavigationModel();
	}

	ImGui::Begin("Navigation");
	switch(status) {
	case NavWorkHorizontalPass:
		ImGui::TextWrapped("Doing navigation horizontal pass");
		break;
	default:
	{
		ImGui::SliderFloat("Max voxel angle on land", &m_max_slope_on_land, 0.0f, 360.0f);
		ImGui::SliderInt("Step size", &m_step_size, 1, 50);
		if(ImGui::Button("Calculate Navigation")) {
			std::thread t(&Navigation::CalculateGraph, this, z_model->GetAABBMin(), z_model->GetAABBMax());
			//std::thread t(&Navigation::CalculateGraph, this, glm::vec3(-200.0f, -200.0f, -200.0f), glm::vec3(200.0f, 200.0f, 200.0f));
			t.detach();
		}
	}
	}
	ImGui::End();
}
