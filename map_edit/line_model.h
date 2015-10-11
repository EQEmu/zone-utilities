#ifndef EQEMU_MAP_EDIT_LINES_MODEL_H
#define EQEMU_MAP_EDIT_LINES_MODEL_H

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "entity.h"

class LineModel : public Entity
{
public:
	LineModel();
	virtual ~LineModel();

	virtual void Draw();
	void Update();
	void AddBox(const glm::vec3& min, const glm::vec3& max);
	void AddLine(const glm::vec3& p0, const glm::vec3& p1);
	void AddTriangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2);
	void Clear();
private:
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ib;

	std::vector<glm::vec3> m_verts;
	std::vector<unsigned int> m_inds;
};

#endif
