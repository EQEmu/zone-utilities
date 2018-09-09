#pragma once

#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct ActorVertex
{
	ActorVertex(float x, float y, float z, float i, float j, float k) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->i = i;
		this->j = j;
		this->k = k;
	}

	float x, y, z;
	float i, j, k;
};

class Actor
{
public:
	Actor();
	~Actor();

	void Draw();
	void Compile();

	void SetLocation(float x, float y, float z) { _x = x; _y = y; _z = z; }
	std::vector<ActorVertex>& GetVerts() { return _verts; }
	std::vector<unsigned int>& GetIndices() { return _inds; }
private:
	GLuint _vao;
	GLuint _vbo;
	GLuint _ibo;
	
	std::vector<ActorVertex> _verts;
	std::vector<unsigned int> _inds;
	
	float _x;
	float _y;
	float _z;
};