#include "actor.h"

Actor::Actor()
{
	_vao = 0;
	_vbo = 0;
	_ibo = 0;
	_x = 0.0f;
	_y = 0.0f;
	_z = 0.0f;
}

Actor::~Actor()
{
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
	}

	if (_ibo) {
		glDeleteBuffers(1, &_ibo);
	}

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
	}
}

void Actor::Draw()
{
	if (_vao) {
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, (GLsizei)_inds.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}

void Actor::Compile()
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _verts.size() * sizeof(ActorVertex), &_verts[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex), 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ActorVertex), (void*)(sizeof(float) * 3));

	glGenBuffers(1, &_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _inds.size() * sizeof(unsigned int), &_inds[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}
