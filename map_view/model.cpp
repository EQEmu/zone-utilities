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
	if(positions.size() == 0 || indices.size() == 0) {
		return;
	}

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

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);
	
	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	for(auto &vert : positions) {
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
	
void Model::Draw(int type) {
	if(!vao) {
		return;
	}

	glBindVertexArray(vao);
	glDrawElements(type, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

DynamicModel::DynamicModel() {
	vao = 0;
	vbo = 0;
	ib = 0;
	compiled_verts = 0;
	compiled_ind = 0;
}

DynamicModel::~DynamicModel() {
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

void DynamicModel::Draw(int type) {
	if(!vao) {
		return;
	}

	glBindVertexArray(vao);
	glDrawElements(type, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void DynamicModel::Compile() {
	if(positions.size() == 0 || indices.size() == 0) {
		return;
	}

	if(vao && positions.size() == compiled_verts && indices.size() == compiled_ind) {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), &positions[0]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(unsigned int), &indices[0]);

		glBindVertexArray(0);
		return;
	}
	
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
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)nullptr);

	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
	
	for(auto &vert : positions) {
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
	compiled_verts = positions.size();
	compiled_ind = indices.size();
}
