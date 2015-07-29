#ifndef EQEMU_MODEL_H
#define EQEMU_MODEL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <vector>

class Model
{
public:
	Model();
	~Model();
	
	void Draw();
	void Compile();
	
	std::vector<glm::vec3>& GetPositions() { return positions; }
	std::vector<unsigned int>& GetIndicies() { return indices; }

	const glm::vec3& GetAABBMin() { return min; }
	const glm::vec3& GetAABBMax() { return max; }
private:
	std::vector<glm::vec3> positions;
	std::vector<unsigned int> indices;
	GLuint vao; //vertex array object
	GLuint vbo; // vertex buffer
	GLuint ib; //index buffer

	glm::vec3 min;
	glm::vec3 max;
};

#endif
