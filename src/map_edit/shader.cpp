//std
#include <stdio.h>

//local
#include "shader.h"

ShaderUniform::ShaderUniform(GLuint location_id) {
	this->location_id = location_id;
}

ShaderUniform::~ShaderUniform() {

}

ShaderProgram::ShaderProgram(const std::string& vertex, const std::string& fragment, const std::string& geometry) {
	program_id = glCreateProgram();
	free_program = true;
	GLuint vertex_shader_id = 0;
	GLuint fragment_shader_id = 0;
	GLuint geometry_shader_id = 0;
	if(vertex.length() > 0) {
		vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
		
		const char *data = &vertex[0];
		glShaderSource(vertex_shader_id, 1, &data, NULL);
		glCompileShader(vertex_shader_id);
		
		int log_len = 0;
		glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &log_len);
		if(log_len > 0) {
			vertex_log.resize(log_len + 1);
			glGetShaderInfoLog(vertex_shader_id, log_len, NULL, &vertex_log[0]);
		}
	}
	
	if(fragment.length() > 0) {
		fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

		const char *data = &fragment[0];
		glShaderSource(fragment_shader_id, 1, &data, NULL);
		glCompileShader(fragment_shader_id);

		int log_len = 0;
		glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len > 0) {
			fragment_log.resize(log_len + 1);
			glGetShaderInfoLog(fragment_shader_id, log_len, NULL, &fragment_log[0]);
		}
	}
	
	if(geometry.length() > 0) {
		geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);

		const char *data = &geometry[0];
		glShaderSource(geometry_shader_id, 1, &data, NULL);
		glCompileShader(geometry_shader_id);

		int log_len = 0;
		glGetShaderiv(geometry_shader_id, GL_INFO_LOG_LENGTH, &log_len);
		if (log_len > 0) {
			geometry_log.resize(log_len + 1);
			glGetShaderInfoLog(geometry_shader_id, log_len, NULL, &geometry_log[0]);
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

ShaderProgram::ShaderProgram(GLuint id) {
	program_id = id;
	free_program = false;
}

ShaderProgram::~ShaderProgram() {
	if(free_program)
		glDeleteProgram(program_id);
}

void ShaderProgram::Use() {
	glUseProgram(program_id);
}

ShaderUniform ShaderProgram::GetUniformLocation(const char *name) {
	GLuint location_id = glGetUniformLocation(program_id, name);
	return ShaderUniform(location_id);
}

ShaderProgram ShaderProgram::Current() {
	GLint current_shader;
	glGetIntegerv(GL_CURRENT_PROGRAM, &current_shader);
	return ShaderProgram(current_shader);
}
