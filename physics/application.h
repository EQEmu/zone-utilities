#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm.hpp>
#include <string>
#include "shader.h"
#include "actor.h"

class Application
{
public:
	Application();
	~Application();
	void Init(GLFWwindow *win);
	void Tick();
	void Render();
	void Resize(int width, int height);

private:
	GLFWwindow *_window;
	int _width;
	int _height;
	bool _show_open;
	bool _show_options;
	bool _show_debug;

	void RenderMenu();
	void RenderUI();
	void ProcessApplicationInput();

	void LoadScene(const std::string &zone_name);

	glm::mat4 _camera_view;
	glm::mat4 _camera_proj;
	glm::vec3 _camera_loc;
	float _hor_angle;
	float _ver_angle;
	float _fov;
	float _near_clip;
	float _far_clip;
	double _last_time;
	bool _right_was_down;

	std::unique_ptr<ShaderProgram> _shader;
	ShaderUniform _model;
	ShaderUniform _view;
	ShaderUniform _proj;

	std::unique_ptr<Actor> _actor;
};