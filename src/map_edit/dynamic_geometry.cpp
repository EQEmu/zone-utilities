#include "dynamic_geometry.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "shader.h"

DynamicGeometry::DynamicGeometry() {
	m_vao = 0;
	m_vbo = 0;
	m_vcbo = 0;
	m_ib = 0;
	m_draw_type = GL_TRIANGLES;
	m_line_width = 1.0f;
	m_depth_write_enabled = true;
	m_depth_test_enabled = true;
	m_blend_enabled = false;
	m_dp = false;
}

DynamicGeometry::~DynamicGeometry() {
	if(m_vbo) {
		glDeleteBuffers(1, &m_vbo);
	}

	if (m_vcbo) {
		glDeleteBuffers(1, &m_vcbo);
	}

	if(m_ib) {
		glDeleteBuffers(1, &m_ib);
	}

	if(m_vao) {
		glDeleteVertexArrays(1, &m_vao);
	}
}

void DynamicGeometry::Draw() {
	if(m_vao) {
		if (!m_depth_write_enabled) {
			glDepthMask(GL_FALSE);
		}

		if (!m_depth_test_enabled) {
			glDisable(GL_DEPTH_TEST);
		}

		if (m_blend_enabled) {
			glEnable(GL_BLEND);
		}

		if(m_line_width != 1.0f)
			glLineWidth(m_line_width);

		glBindVertexArray(m_vao);
		glDrawElements(m_draw_type, (GLsizei)m_inds.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		if (m_dp) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			glm::vec4 temp_tint;
			temp_tint.x = 0.0f;
			temp_tint.y = 0.0f;
			temp_tint.z = 0.0f;
			temp_tint.w = 1.0f;

			auto shader = ShaderProgram::Current();
			auto tint = shader.GetUniformLocation("Tint");
			tint.SetValuePtr4(1, &temp_tint[0]);

			glBindVertexArray(m_vao);
			glDrawElements(m_draw_type, (GLsizei)m_inds.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (m_line_width != 1.0f)
			glLineWidth(1.0f);

		if (!m_depth_write_enabled) {
			glDepthMask(GL_TRUE);
		}

		if (!m_depth_test_enabled) {
			glEnable(GL_DEPTH_TEST);
		}

		if (m_blend_enabled) {
			glDisable(GL_BLEND);
		}
	}
}

void DynamicGeometry::GetCollisionMesh(std::vector<glm::vec3>& verts, std::vector<unsigned int>& inds)
{
	verts = m_verts;
	inds = m_inds;
}

void DynamicGeometry::Clear()
{
	m_verts.clear();
	m_inds.clear();
	m_vert_colors.clear();
}

void DynamicGeometry::Update() {
	if (m_verts.size() == 0 || m_inds.size() == 0 || m_vert_colors.size() == 0) {
		return;
	}

	if (m_vao) {
		glBindVertexArray(m_vao);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(glm::vec3), &m_verts[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, m_vcbo);
		glBufferData(GL_ARRAY_BUFFER, m_vert_colors.size() * sizeof(glm::vec3), &m_vert_colors[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_inds.size() * sizeof(unsigned int), &m_inds[0], GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
		CalcBB();
		return;
	}

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_verts.size() * sizeof(glm::vec3), &m_verts[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glGenBuffers(1, &m_vcbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vcbo);
	glBufferData(GL_ARRAY_BUFFER, m_vert_colors.size() * sizeof(glm::vec3), &m_vert_colors[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glGenBuffers(1, &m_ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_inds.size() * sizeof(unsigned int), &m_inds[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
	CalcBB();
}

void DynamicGeometry::AddVertex(const glm::vec3 & p, const glm::vec3 & color)
{
	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(p);
	m_vert_colors.push_back(color);
	m_inds.push_back(base_index);
}

void DynamicGeometry::AddLineBox(const glm::vec3 & min, const glm::vec3 & max, const glm::vec3& color)
{
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

	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);

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

void DynamicGeometry::AddLine(const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3& color)
{
	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(p0);
	m_verts.push_back(p1);

	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 1);
}

void DynamicGeometry::AddTriangle(const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3 & p2, const glm::vec3& color)
{
	unsigned int base_index = (unsigned int)m_verts.size();
	m_verts.push_back(p0);
	m_verts.push_back(p1);
	m_verts.push_back(p2);

	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);

	m_inds.push_back(base_index + 0);
	m_inds.push_back(base_index + 1);
	m_inds.push_back(base_index + 2);
}

void DynamicGeometry::AddLineCylinder(const glm::vec3 &min, const glm::vec3 &max, const glm::vec3 &color)
{
	//AddVertex
	static const int NUM_SEG = 16;
	static float dir[NUM_SEG * 2];
	static bool init = false;
	if (!init)
	{
		init = true;
		for (int i = 0; i < NUM_SEG; ++i)
		{
			const float a = (float)i / (float)NUM_SEG * (float)M_PI * 2.0f;
			dir[i * 2] = cosf(a);
			dir[i * 2 + 1] = sinf(a);
		}
	}

	const float cx = (max.x + min.x) / 2;
	const float cz = (max.z + min.z) / 2;
	const float rx = (max.x - min.x) / 2;
	const float rz = (max.z - min.z) / 2;

	for (int i = 0, j = NUM_SEG - 1; i < NUM_SEG; j = i++)
	{
		AddVertex(glm::vec3(cx + dir[j * 2 + 0] * rx, min.y, cz + dir[j * 2 + 1] * rz), color);
		AddVertex(glm::vec3(cx + dir[i * 2 + 0] * rx, min.y, cz + dir[i * 2 + 1] * rz), color);
		AddVertex(glm::vec3(cx + dir[j * 2 + 0] * rx, max.y, cz + dir[j * 2 + 1] * rz), color);
		AddVertex(glm::vec3(cx + dir[i * 2 + 0] * rx, max.y, cz + dir[i * 2 + 1] * rz), color);
	}
	for (int i = 0; i < NUM_SEG; i += NUM_SEG / 4)
	{
		AddVertex(glm::vec3(cx + dir[i * 2 + 0] * rx, min.y, cz + dir[i * 2 + 1] * rz), color);
		AddVertex(glm::vec3(cx + dir[i * 2 + 0] * rx, max.y, cz + dir[i * 2 + 1] * rz), color);
	}
}

void DynamicGeometry::AddLineArrow(const glm::vec3 &p0, const glm::vec3 &p1, float size, const glm::vec3 &color)
{
	AddLine(p0, p1, color);
	AddLineArrowHead(p1, p0, size, color);
}

void DynamicGeometry::AddLineArrowHead(const glm::vec3 &p0, const glm::vec3 &p1, float size, const glm::vec3 &color)
{
	const float eps = 0.001f;
	if (glm::dot(p0, p1) < eps * eps)
		return;

	glm::vec3 ax;
	glm::vec3 ay(0.0, 1.0, 0.0);
	glm::vec3 az;

	az = p0 - p1;
	az = glm::normalize(az);

	ax = glm::cross(ay, az);
	ay = glm::cross(az, ax);
	ay = glm::normalize(ay);

	AddLine(p0, glm::vec3(p0[0] + az[0] * size + ax[0] * size / 3, p0[1] + az[1] * size + ax[1] * size / 3, p0[2] + az[2] * size + ax[2] * size / 3), color);
	AddLine(p0, glm::vec3(p0[0] + az[0] * size - ax[0] * size / 3, p0[1] + az[1] * size - ax[1] * size / 3, p0[2] + az[2] * size - ax[2] * size / 3), color);
}

void DynamicGeometry::AddBox(const glm::vec3 &min, const glm::vec3 &max, const glm::vec3 & color)
{
	glm::vec3 v1(min.x, max.y, min.z);
	glm::vec3 v2(min.x, max.y, max.z);
	glm::vec3 v3(max.x, max.y, max.z);
	glm::vec3 v4(max.x, max.y, min.z);
	glm::vec3 v5(min.x, min.y, min.z);
	glm::vec3 v6(min.x, min.y, max.z);
	glm::vec3 v7(max.x, min.y, max.z);
	glm::vec3 v8(max.x, min.y, min.z);

	uint32_t current_index = (uint32_t)m_verts.size();
	m_verts.push_back(v1);
	m_verts.push_back(v2);
	m_verts.push_back(v3);
	m_verts.push_back(v4);
	m_verts.push_back(v5);
	m_verts.push_back(v6);
	m_verts.push_back(v7);
	m_verts.push_back(v8);

	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);
	m_vert_colors.push_back(color);

	//top
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 0);

	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 2);

	//back
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 1);

	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 6);

	//bottom
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 4);

	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 6);

	//front
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 0);

	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 7);

	//left
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 0);

	m_inds.push_back(current_index + 5);
	m_inds.push_back(current_index + 1);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 0);
	m_inds.push_back(current_index + 4);
	m_inds.push_back(current_index + 5);

	//right
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 3);

	m_inds.push_back(current_index + 6);
	m_inds.push_back(current_index + 2);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 3);
	m_inds.push_back(current_index + 7);
	m_inds.push_back(current_index + 6);
}

void DynamicGeometry::CalcBB()
{
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
