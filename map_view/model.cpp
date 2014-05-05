#include "model.h"

Model::Model() {
	vao = 0;
	vbo[0] = 0;
	vbo[1] = 0;
	ib = 0;
}

Model::~Model() {
	if(vbo[0]) {
		glDeleteBuffers(1, &vbo[0]);
	}
	
	if(vbo[1]) {
		glDeleteBuffers(1, &vbo[1]);
	}
	
	if(ib) {
		glDeleteBuffers(1, &ib);
	}
	
	if(vao) {
		glDeleteVertexArrays(1, &vao);
	}
}

void Model::Compile() {
	if(vbo[0]) {
		glDeleteBuffers(1, &vbo[0]);
		vbo[0] = 0;
	}
	
	if(vbo[1]) {
		glDeleteBuffers(1, &vbo[1]);
		vbo[1] = 0;
	}
	
	if(ib) {
		glDeleteBuffers(1, &ib);
		ib = 0;
	}
	
	if(vao) {
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &vbo[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBindVertexArray(0);
}
	
void Model::Draw() {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
