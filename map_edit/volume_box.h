#ifndef EQEMU_MAP_EDIT_VOLUME_BOX_H
#define EQEMU_MAP_EDIT_VOLUME_BOX_H

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "entity.h"

class VolumeBox : public Entity
{
public:
	VolumeBox();
	virtual ~VolumeBox();

	virtual void Draw();
	void Update();
	void AddBox(const glm::vec3& min, const glm::vec3& max);
	void Clear();
private:
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ib;

	std::vector<glm::vec3> m_verts;
	std::vector<unsigned int> m_inds;
};

#endif
