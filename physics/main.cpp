#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "imgui_glfw.h"
#include <event/event_loop.h>
#include <dependency/container.h>
#include <log/composite_logger.h>
#include <log/console_logger.h>
#include <log/file_logger.h>
#include <core/config.h>
#include "renderer.h"

std::unique_ptr<Renderer> renderer;

void setup_dependencies() {
	auto config = EQEmu::Container::Get().RegisterSingleton<EQEmu::IConfig, EQEmu::Config>();
	auto logger = new EQEmu::CompositeLogger();

	logger->Add(new EQEmu::ConsoleLogger());
	logger->Add(new EQEmu::FileLogger("physics.log"));

	if (config->GetLogEnabled("Critical", true)) {
		logger->Enable(EQEmu::LogCritical);
	}

	if (config->GetLogEnabled("Error", true)) {
		logger->Enable(EQEmu::LogError);
	}

	if (config->GetLogEnabled("Debug", true)) {
		logger->Enable(EQEmu::LogDebug);
	}

	if (config->GetLogEnabled("Warning", true)) {
		logger->Enable(EQEmu::LogWarning);
	}

	if (config->GetLogEnabled("Info", true)) {
		logger->Enable(EQEmu::LogInfo);
	}

	if (config->GetLogEnabled("Trace", false)) {
		logger->Enable(EQEmu::LogTrace);
	}

	EQEmu::Container::Get().RegisterInstance<EQEmu::ILogger>(logger);
}

int main(int argc, char **argv)
{
	setup_dependencies();

	auto logger = EQEmu::Container::Get().Resolve<EQEmu::ILogger>();

	if (!glfwInit()) {
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
	if (!win) {
		logger->LogCritical("Couldn't create an OpenGL window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		logger->LogCritical("Couldn't init glew.");
		glfwTerminate();
		return -1;
	}

	renderer.reset(new Renderer());
	renderer->Init(win);

	glfwSetFramebufferSizeCallback(win, [](GLFWwindow *win, int width, int height) {
		renderer->Resize(width, height);
	});

	ImGui_ImplGlfwGL3_Init(win, true);
	while (!glfwWindowShouldClose(win)) {
		renderer->Tick();
		renderer->Render();
		EQ::EventLoop::Get().Process();
	}

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}
