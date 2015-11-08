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
	void Update();

	std::vector<glm::vec3>& GetVerts() { return m_verts; }
	std::vector<unsigned int>& GetInds() { return m_inds; }

	const glm::vec3& GetAABBMin() { return m_min; }
	const glm::vec3& GetAABBMax() { return m_max; }

	void SetDrawType(GLenum dt) { m_draw_type = dt; }
private:
	void CalcBB();
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ib;

	std::vector<glm::vec3> m_verts;
	std::vector<unsigned int> m_inds;

	glm::vec3 m_min;
	glm::vec3 m_max;

	GLenum m_draw_type;
};

#endif
