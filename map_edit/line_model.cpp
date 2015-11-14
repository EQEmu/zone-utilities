#include "line_model.h"
#include "shader.h"

LineModel::LineModel() {
	m_line_width = 1.0f;
	m_depth_enabled = true;
	m_vao = 0;
	m_vbo = 0;
	m_ib = 0;
	m_tint = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

LineModel::~LineModel() {
	if(m_vbo) {
		glDeleteBuffers(1, &m_vbo);
	}

	if(m_ib) {
		glDeleteBuffers(1, &m_ib);
	}

	if(m_vao) {
		glDeleteVertexArrays(1, &m_vao);
	}
}

void LineModel::Draw() {
	if(m_vao) {
		if (!m_depth_enabled) {
			glDepthMask(GL_FALSE);
		}

		glLineWidth(m_line_width);

		ShaderProgram shader = ShaderProgram::Current();
		auto tint = shader.GetUniformLocation("Tint");
		tint.SetValuePtr4(1, &m_tint[0]);

		glBindVertexArray(m_vao);
		glDrawElements(GL_LINES, (GLsizei)m_inds.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glLineWidth(1.0f);

		if (!m_depth_enabled) {
			glDepthMask(GL_TRUE);
		}
	}
}

void LineModel::Update() {
	if (m_verts.size() == 0 || m_inds.size() == 0) {
		return;
	}

	if(m_vao) {
		glBindVertexArray(m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(glm::vec3), &m_verts[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_inds.size() * sizeof(unsigned int), &m_inds[0], GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(glm::vec3), &m_verts[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_inds.size() * sizeof(unsigned int), &m_inds[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}

void LineModel::AddBox(const glm::vec3& min, const glm::vec3& max) {
	glm::vec3 v1(min.x, min.y, min.z);
	glm::vec3 v2(max.x, min.y, min.z);
	glm::vec3 v3(max.x, min.y, max.z);
	glm::vec3 v4(min.x, min.y, max.z);
	glm::vec3 v5(min.x, max.y, min.z);
	glm::vec3 v6(max.x, max.y, min.z);
	glm::vec3 v7(max.x, max.y, max.z);
	glm::vec3 v8(min.x, max.y, max.z);

	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(v1);
	m_verts.push_back(v2);
	m_verts.push_back(v3);
	m_verts.push_back(v4);
	m_verts.push_back(v5);
	m_verts.push_back(v6);
	m_verts.push_back(v7);
	m_verts.push_back(v8);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 1);
	m_inds.push_back(base_index + 1);
	m_inds.push_back(base_index + 2);
	m_inds.push_back(base_index + 2);
	m_inds.push_back(base_index + 3);
	m_inds.push_back(base_index + 3);
	m_inds.push_back(base_index + 0);

	m_inds.push_back(base_index + 4);
	m_inds.push_back(base_index + 5);
	m_inds.push_back(base_index + 5);
	m_inds.push_back(base_index + 6);
	m_inds.push_back(base_index + 6);
	m_inds.push_back(base_index + 7);
	m_inds.push_back(base_index + 7);
	m_inds.push_back(base_index + 4);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 4);
	m_inds.push_back(base_index + 1);
	m_inds.push_back(base_index + 5);
	m_inds.push_back(base_index + 2);
	m_inds.push_back(base_index + 6);
	m_inds.push_back(base_index + 3);
	m_inds.push_back(base_index + 7);
}

void LineModel::AddLine(const glm::vec3& p0, const glm::vec3& p1) {
	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(p0);
	m_verts.push_back(p1);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 1);
}

void LineModel::AddTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2) {
	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(p0);
	m_verts.push_back(p1);
	m_verts.push_back(p2);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 1);
	m_inds.push_back(base_index + 2);
}

void LineModel::Clear() {
	m_verts.clear();
	m_inds.clear();
}
