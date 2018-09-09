#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class ShaderUniform
{
public:
	ShaderUniform() { }
	~ShaderUniform();

	inline void SetValue(float v) { glUniform1f(location_id, v); }
	inline void SetValue(float v, float v2) { glUniform2f(location_id, v, v2); }
	inline void SetValue(float v, float v2, float v3) { glUniform3f(location_id, v, v2, v3); }
	inline void SetValue(float v, float v2, float v3, float v4) { glUniform4f(location_id, v, v2, v3, v4); }
	inline void SetValue(int v) { glUniform1i(location_id, v); }
	inline void SetValue(int v, int v2) { glUniform2i(location_id, v, v2); }
	inline void SetValue(int v, int v2, int v3) { glUniform3i(location_id, v, v2, v3); }
	inline void SetValue(int v, int v2, int v3, int v4) { glUniform4i(location_id, v, v2, v3, v4); }
	inline void SetValue(unsigned int v) { glUniform1ui(location_id, v); }
	inline void SetValue(unsigned int v, unsigned int v2) { glUniform2ui(location_id, v, v2); }
	inline void SetValue(unsigned int v, unsigned int v2, unsigned int v3) { glUniform3ui(location_id, v, v2, v3); }
	inline void SetValue(unsigned int v, unsigned int v2, unsigned int v3, unsigned int v4) { glUniform4ui(location_id, v, v2, v3, v4); }

	inline void SetValuePtr1(int count, float *v) { glUniform1fv(location_id, count, v); }
	inline void SetValuePtr2(int count, float *v) { glUniform2fv(location_id, count, v); }
	inline void SetValuePtr3(int count, float *v) { glUniform3fv(location_id, count, v); }
	inline void SetValuePtr4(int count, float *v) { glUniform4fv(location_id, count, v); }
	inline void SetValuePtr1(int count, int *v) { glUniform1iv(location_id, count, v); }
	inline void SetValuePtr2(int count, int *v) { glUniform2iv(location_id, count, v); }
	inline void SetValuePtr3(int count, int *v) { glUniform3iv(location_id, count, v); }
	inline void SetValuePtr4(int count, int *v) { glUniform4iv(location_id, count, v); }
	inline void SetValuePtr1(int count, unsigned int *v) { glUniform1uiv(location_id, count, v); }
	inline void SetValuePtr2(int count, unsigned int *v) { glUniform2uiv(location_id, count, v); }
	inline void SetValuePtr3(int count, unsigned int *v) { glUniform3uiv(location_id, count, v); }
	inline void SetValuePtr4(int count, unsigned int *v) { glUniform4uiv(location_id, count, v); }

	inline void SetValueMatrix2(int count, bool transpose, float *v) { glUniformMatrix2fv(location_id, count, transpose, v); }
	inline void SetValueMatrix3(int count, bool transpose, float *v) { glUniformMatrix3fv(location_id, count, transpose, v); }
	inline void SetValueMatrix4(int count, bool transpose, float *v) { glUniformMatrix4fv(location_id, count, transpose, v); }
	inline void SetValueMatrix2x3(int count, bool transpose, float *v) { glUniformMatrix2x3fv(location_id, count, transpose, v); }
	inline void SetValueMatrix3x2(int count, bool transpose, float *v) { glUniformMatrix3x2fv(location_id, count, transpose, v); }
	inline void SetValueMatrix2x4(int count, bool transpose, float *v) { glUniformMatrix2x4fv(location_id, count, transpose, v); }
	inline void SetValueMatrix4x2(int count, bool transpose, float *v) { glUniformMatrix4x2fv(location_id, count, transpose, v); }
	inline void SetValueMatrix3x4(int count, bool transpose, float *v) { glUniformMatrix3x4fv(location_id, count, transpose, v); }
	inline void SetValueMatrix4x3(int count, bool transpose, float *v) { glUniformMatrix4x3fv(location_id, count, transpose, v); }

private:
	ShaderUniform(GLuint location_id);
	GLuint location_id;

	friend class ShaderProgram;
};

class ShaderProgram
{
public:
	ShaderProgram(const std::string& vertex = "", const std::string& fragment = "", const std::string& geometry = "");
	ShaderProgram(GLuint id);
	~ShaderProgram();

	void Use();
	const std::string &GetVertexShaderLog() { return vertex_log; }
	const std::string &GetFragmentShaderLog() { return fragment_log; }
	const std::string &GetGeometryShaderLog() { return geometry_log; }
	const std::string &GetLinkLog() { return link_log; }

	ShaderUniform GetUniformLocation(const char *name);

	static ShaderProgram Current();
private:
	GLuint program_id;
	bool free_program;
	std::string vertex_log;
	std::string fragment_log;
	std::string geometry_log;
	std::string link_log;
};
