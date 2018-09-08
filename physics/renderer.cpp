#include "renderer.h"
#include "imgui_glfw.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Init(GLFWwindow *win)
{
	_window = win;
	glfwGetFramebufferSize(win, &_width, &_height);
}

void Renderer::Tick()
{
	glfwPollEvents();
}

void Renderer::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//ImGui_ImplGlfwGL3_NewFrame();
	//
	//ImGui::Render();
	glfwSwapBuffers(_window);
}

void Renderer::Resize(int width, int height)
{
	_width = width;
	_height = height;
}
