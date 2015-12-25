#ifndef EQEMU_MAP_EDIT_DYNAMIC_GEOMETRY_H
#define EQEMU_MAP_EDIT_DYNAMIC_GEOMETRY_H

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "entity.h"

class DynamicGeometry : public Entity
{
public:
	DynamicGeometry();
	virtual ~DynamicGeometry();

	virtual void Draw();
	void Clear();
	void Update();

	std::vector<glm::vec3>& GetVerts() { return m_verts; }
	std::vector<glm::vec3>& GetVertColors() { return m_vert_colors; }
	std::vector<unsigned int>& GetInds() { return m_inds; }

	const glm::vec3& GetAABBMin() { return m_min; }
	const glm::vec3& GetAABBMax() { return m_max; }

	void SetDrawType(GLenum dt) { m_draw_type = dt; }
	void SetDepthWriteEnabled(bool v) { m_depth_write_enabled = v; }
	void SetDepthTestEnabled(bool v) { m_depth_test_enabled = v; }
	void SetBlend(bool v) { m_blend_enabled = v; }
	void SetLineWidth(float f) { m_line_width = f; }
	void SetDoublePass(bool dp) { m_dp = dp; }

	void AddVertex(const glm::vec3& p, const glm::vec3& color);
	void AddLineBox(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
	void AddLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
	void AddTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
	void AddLineCylinder(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
	void AddLineArrow(const glm::vec3& p0, const glm::vec3& p1, float size, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
	void AddLineArrowHead(const glm::vec3& p0, const glm::vec3& p1, float size, const glm::vec3& color = glm::vec3(1.0, 1.0, 1.0));
private:
	void CalcBB();
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_vcbo;
	GLuint m_ib;

	std::vector<glm::vec3> m_verts;
	std::vector<glm::vec3> m_vert_colors;
	std::vector<unsigned int> m_inds;

	glm::vec3 m_min;
	glm::vec3 m_max;

	float m_line_width;
	bool m_depth_write_enabled;
	bool m_depth_test_enabled;
	bool m_blend_enabled;
	GLenum m_draw_type;
	bool m_dp;
};

#endif
