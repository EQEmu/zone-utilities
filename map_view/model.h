#ifndef EQEMU_MODEL_H
#define EQEMU_MODEL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <vector>


class Model
{
public:
	Model();
	virtual ~Model();
	
	virtual void Draw(int type = GL_TRIANGLES);
	virtual void Compile();
	
	std::vector<glm::vec3>& GetPositions() { return positions; }
	std::vector<unsigned int>& GetIndicies() { return indices; }

	const glm::vec3& GetAABBMin() { return min; }
	const glm::vec3& GetAABBMax() { return max; }
protected:
	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;
	GLuint vao; //vertex array object
	GLuint vbo; // vertex buffer
	GLuint ib; //index buffer

	glm::vec3 min;
	glm::vec3 max;
};

class DynamicModel : public Model
{
public:
	DynamicModel();
	virtual ~DynamicModel();
	
	virtual void Draw(int type = GL_TRIANGLES);
	virtual void Compile();
protected:
	size_t compiled_verts;
	size_t compiled_ind;
};

#endif
