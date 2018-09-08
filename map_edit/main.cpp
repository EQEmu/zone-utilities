//std
#include <memory>

//local
#include "imgui_glfw.h"
#include "scene.h"
#include "module_navigation.h"
#include "module_wp.h"
#include "module_volume.h"
#include "event/event_loop.h"
#include "dependency/container.h"
#include "log/composite_logger.h"
#include "log/console_logger.h"
#include "log/file_logger.h"

void setup_dependencies() {
	EQEmu::Container::Get().RegisterSingleton<EQEmu::ILogger, EQEmu::CompositeLogger>();
	auto logger = std::dynamic_pointer_cast<EQEmu::CompositeLogger>(EQEmu::Container::Get().Resolve<EQEmu::ILogger>());

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("awater.log"));
	logger->Enable(EQEmu::LogCritical);
	logger->Enable(EQEmu::LogError);
	logger->Enable(EQEmu::LogDebug);
	logger->Enable(EQEmu::LogWarning);
	logger->Enable(EQEmu::LogInfo);
}

std::unique_ptr<Scene> scene;

int main(int argc, char **argv)
{
	setup_dependencies();

	auto logger = EQEmu::Container::Get().Resolve<EQEmu::ILogger>();

	if(!glfwInit()) {
		logger->LogCritical("Couldn't init graphical system.");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


	GLFWwindow *win = glfwCreateWindow(mode->width, mode->height, "Map Edit", nullptr, nullptr);
	if(!win) {
		logger->LogCritical("Couldn't create an OpenGL window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		logger->LogCritical("Couldn't init glew.");
		glfwTerminate();
		return -1;
	}

	scene.reset(new Scene());
	scene->RegisterModule(new ModuleNavigation());
	scene->RegisterModule(new ModuleVolume());
	scene->RegisterModule(new ModuleWP());
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
