#include <memory>

#include <graphics/imgui_glfw.h>
#include <event/event_loop.h>
#include <log_macros.h>
#include <log_stdout.h>
#include <log_file.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main(int argc, char **argv)
{
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogFile("model_view.log")));

	if (!glfwInit()) {
		eqLogMessage(LogFatal, "Couldn't init graphical system.");
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

	GLFWwindow *win = glfwCreateWindow(mode->width, mode->height, "Model Viewer", nullptr, nullptr);
	if (!win) {
		eqLogMessage(LogFatal, "Couldn't create an OpenGL window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		eqLogMessage(LogFatal, "Couldn't init glew.");
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(win, [](GLFWwindow *win, int width, int height) {
	});

	ImGui_ImplGlfwGL3_Init(win, true);
	while (!glfwWindowShouldClose(win)) {
		EQ::EventLoop::Get().Process();
	}

	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	return 0;
}