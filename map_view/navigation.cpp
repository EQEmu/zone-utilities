#include "navigation.h"
#include "imgui.h"
#include "oriented_bounding_box.h"

void Navigation::BuildNodeModel() {
	m_nodes_model.reset(new Model());

	auto &positions = m_nodes_model->GetPositions();
	auto &indicies = m_nodes_model->GetIndicies();

	m_nodes->TraverseSelection(m_bounds, [this, &positions, &indicies](const glm::vec3 &pos, Node *ent) {
		float extent = 1.0f;
		float scale = 1.0f;

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
		transformation = CreateTranslateMatrix(pos.y, pos.x, pos.z) * transformation;

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
	});

	m_nodes_model->Compile();
}

void Navigation::AddNode(float x, float y, float z) {
	Node *n = new Node();
	n->x = 0;
	n->y = 0;
	n->z = 0;

	for(int i = 0; i < 8; ++i) {
		n->connected[i] = nullptr;
	}

	m_nodes->Insert(glm::vec3(x, y, z), n);
	m_existing_nodes.push_back(std::unique_ptr<Node>(n));
}