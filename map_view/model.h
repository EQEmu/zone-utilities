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
	std::vector<glm::vec3>& GetColors() { return colors; }
	std::vector<unsigned int>& GetIndicies() { return indices; }
private:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> colors;
	std::vector<unsigned int> indices;
	GLuint vao; //vertex array object
	GLuint vbo[2]; // vertex buffer, position, color
	GLuint ib; //index buffer
};

#endif
