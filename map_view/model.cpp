#include "model.h"

Model::Model() {
	vao = 0;
	vbo = 0;
	ib = 0;
}

Model::~Model() {
	if(vbo) {
		glDeleteBuffers(1, &vbo);
	}
	
	if(ib) {
		glDeleteBuffers(1, &ib);
	}
	
	if(vao) {
		glDeleteVertexArrays(1, &vao);
	}
}

void Model::Compile() {
	if(vbo) {
		glDeleteBuffers(1, &vbo);
		vbo = 0;
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
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBindVertexArray(0);

	for(auto &vert : this->positions) {
		if(vert.x < min.x) {
			min.x = vert.x;
		}

		if(vert.y < min.y) {
			min.y = vert.y;
		}

		if(vert.z < min.z) {
			min.z = vert.z;
		}

		if(vert.x > max.x) {
			max.x = vert.x;
		}

		if(vert.y > max.y) {
			max.y = vert.y;
		}

		if(vert.z > max.z) {
			max.z = vert.z;
		}
	}
}
	
void Model::Draw() {
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
