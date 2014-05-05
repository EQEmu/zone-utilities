#include <stdio.h>
#include "shader.h"

ShaderUniform::ShaderUniform(GLuint location_id) {
	this->location_id = location_id;
}

ShaderUniform::~ShaderUniform() {

}

ShaderProgram::ShaderProgram(std::string vertex, std::string fragment, std::string geometry) {
	program_id = glCreateProgram();
	GLuint vertex_shader_id = 0;
	GLuint fragment_shader_id = 0;
	GLuint geometry_shader_id = 0;
	if(vertex.length() > 0) {
		vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
		FILE *f = fopen(vertex.c_str(), "rb");
		if(f) {
			fseek(f, 0, SEEK_END);
			size_t sz = ftell(f);
			rewind(f);
			
			if(sz > 0) {
				char *vertex_data = new char[sz + 1];
				vertex_data[sz] = 0;
				
				size_t frs = 0;
				size_t offset = 0;
				while((frs = fread(&vertex_data[offset], 1, 512, f) == 512)) {
					offset += 512;
				}
				
				const char *vertex_data_ptr = &vertex_data[0];
				glShaderSource(vertex_shader_id, 1, &vertex_data_ptr, NULL);
				glCompileShader(vertex_shader_id);
			
				int log_len = 0;
				glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &log_len);
				if(log_len > 0) {
					vertex_log.resize(log_len + 1);
					glGetShaderInfoLog(vertex_shader_id, log_len, NULL, &vertex_log[0]);
				}
				
				delete[] vertex_data;
			} else {
				glDeleteShader(vertex_shader_id);
				vertex_shader_id = 0;
			}
			
			fclose(f);
		} else {
			vertex_log = "Couldn't open vertex shader file: " + vertex;
		}
	}
	
	if(fragment.length() > 0) {
		fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
		FILE *f = fopen(fragment.c_str(), "rb");
		if(f) {
			fseek(f, 0, SEEK_END);
			size_t sz = ftell(f);
			rewind(f);
			
			if(sz > 0) {
				char *fragment_data = new char[sz + 1];
				fragment_data[sz] = 0;
				
				size_t frs = 0;
				size_t offset = 0;
				while((frs = fread(&fragment_data[offset], 1, 512, f) == 512)) {
					offset += 512;
				}
				
				const char *fragment_data_ptr = &fragment_data[0];
				glShaderSource(fragment_shader_id, 1, &fragment_data_ptr, NULL);
				glCompileShader(fragment_shader_id);
			
				int log_len = 0;
				glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &log_len);
				if(log_len > 0) {
					fragment_log.resize(log_len + 1);
					glGetShaderInfoLog(fragment_shader_id, log_len, NULL, &fragment_log[0]);
				}
				
				delete[] fragment_data;
			} else {
				glDeleteShader(fragment_shader_id);
				fragment_shader_id = 0;
			}
			
			fclose(f);
		} else {
			fragment_log = "Couldn't open fragment shader file: " + fragment;
		}
	}
	
	if(geometry.length() > 0) {
		geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
		FILE *f = fopen(geometry.c_str(), "rb");
		if(f) {
			fseek(f, 0, SEEK_END);
			size_t sz = ftell(f);
			rewind(f);
			
			if(sz > 0) {
				char *geometry_data = new char[sz + 1];
				geometry_data[sz] = 0;
				
				size_t frs = 0;
				size_t offset = 0;
				while((frs = fread(&geometry_data[offset], 1, 512, f) == 512)) {
					offset += 512;
				}
				
				const char *geometry_data_ptr = &geometry_data[0];
				glShaderSource(geometry_shader_id, 1, &geometry_data_ptr, NULL);
				glCompileShader(geometry_shader_id);
			
				int log_len = 0;
				glGetShaderiv(geometry_shader_id, GL_INFO_LOG_LENGTH, &log_len);
				if(log_len > 0) {
					geometry_log.resize(log_len + 1);
					glGetShaderInfoLog(geometry_shader_id, log_len, NULL, &geometry_log[0]);
				}
				
				delete[] geometry_data;
			} else {
				glDeleteShader(geometry_shader_id);
				geometry_shader_id = 0;
			}
			
			fclose(f);
		} else {
			geometry_log = "Couldn't open geometry shader file: " + geometry;
		}
	}
	
	if(vertex_shader_id) {
		glAttachShader(program_id, vertex_shader_id);
	}
	
	if(fragment_shader_id) {
		glAttachShader(program_id, fragment_shader_id);
	}
	
	if(geometry_shader_id) {
		glAttachShader(program_id, geometry_shader_id);
	}
	
	glLinkProgram(program_id);
	
	int log_len = 0;
	glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &log_len);
	if (log_len > 0) {
		link_log.resize(log_len + 1);
		glGetShaderInfoLog(program_id, log_len, NULL, &link_log[0]);
	}
	
	if(vertex_shader_id) {
		glDeleteShader(vertex_shader_id);
	}
	
	if(fragment_shader_id) {
		glDeleteShader(fragment_shader_id);
	}
	
	if(geometry_shader_id) {
		glDeleteShader(geometry_shader_id);
	}
}

ShaderProgram::~ShaderProgram() {
	glDeleteProgram(program_id);
}

void ShaderProgram::Use() {
	glUseProgram(program_id);
}

ShaderUniform ShaderProgram::GetUniformLocation(std::string name) {
	GLuint location_id = glGetUniformLocation(program_id, name.c_str());
	return ShaderUniform(location_id);
}
