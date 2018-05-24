//std
#include <memory>

//local
#include "imgui_glfw.h"
#include "scene.h"
#include "module_navigation.h"
#include "module_wp.h"
#include "module_volume.h"
#include "log_file.h"
#include "event/event_loop.h"

std::unique_ptr<Scene> scene;

int main(int argc, char **argv)
{
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogFile("map_edit.log")));

	if(!glfwInit()) {
		eqLogMessage(LogFatal, "Couldn't init graphical system.");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


	GLFWwindow *win = glfwCreateWindow(mode->width, mode->height, "Map Edit", nullptr, nullptr);
	if(!win) {
		eqLogMessage(LogFatal, "Couldn't create an OpenGL window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		eqLogMessage(LogFatal, "Couldn't init glew.");
		glfwTerminate();
		return -1;
	}

	scene.reset(new Scene());
	scene->RegisterModule(new ModuleNavigation());
	scene->RegisterModule(new ModuleWP());
	scene->RegisterModule(new ModuleVolume());
	scene->Init(win);

	glfwSetFramebufferSizeCallback(win, [](GLFWwindow *win, int width, int height) {
		scene->Resize(width, height);
	});

	ImGui_ImplGlfwGL3_Init(win, true);
	while(!glfwWindowShouldClose(win)) {
		scene->Tick();
		scene->Render();
		EQ::EventLoop::Get().Process();
	}

	scene->UnregisterAllModules();

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
