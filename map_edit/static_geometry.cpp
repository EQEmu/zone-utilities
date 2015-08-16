#include "static_geometry.h"

StaticGeometry::StaticGeometry() {
	m_vao = 0;
	m_vbo = 0;
	m_ib = 0;
	m_draw_type = GL_TRIANGLES;
}

StaticGeometry::~StaticGeometry() {
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

void StaticGeometry::Draw() {
	if(m_vao) {
		glBindVertexArray(m_vao);
		glDrawElements(m_draw_type, (GLsizei)m_inds.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void StaticGeometry::Compile() {
	if(m_vao || m_verts.size() == 0 || m_inds.size() == 0) {
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(glm::vec3), &m_verts[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_inds.size() * sizeof(unsigned int), &m_inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	for(auto &vert : m_verts) {
		if(vert.x < m_min.x) {
			m_min.x = vert.x;
		}

		if(vert.y < m_min.y) {
			m_min.y = vert.y;
		}

		if(vert.z < m_min.z) {
			m_min.z = vert.z;
		}

		if(vert.x > m_max.x) {
			m_max.x = vert.x;
		}

		if(vert.y > m_max.y) {
			m_max.y = vert.y;
		}

		if(vert.z > m_max.z) {
			m_max.z = vert.z;
		}
	}
}
