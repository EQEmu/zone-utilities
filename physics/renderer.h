#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm.hpp>

class Renderer
{
public:
	Renderer();
	~Renderer();
	void Init(GLFWwindow *win);
	void Tick();
	void Render();
	void Resize(int width, int height);

private:
	GLFWwindow *_window;
	int _width;
	int _height;
};