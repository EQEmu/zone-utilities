#include "app.h"
#include <graphics/imgui_glfw.h>
#include <imgui.h>
#include <event/event_loop.h>
#include <log_macros.h>
#include <log_stdout.h>
#include <log_file.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

EQ::Graphics::App *CurrentAppRunning = nullptr;;

EQ::Graphics::App::App(const std::string &logfile) {
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogFile(logfile)));
}

EQ::Graphics::App::~App() {
	
}

int EQ::Graphics::App::Run(const std::string &name) {
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

	glfwWindowHint(GLFW_MAXIMIZED, 1);

	mWindow = glfwCreateWindow(mode->width, mode->height, name.c_str(), nullptr, nullptr);
	if (!mWindow) {
		eqLogMessage(LogFatal, "Couldn't create an OpenGL window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(mWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		eqLogMessage(LogFatal, "Couldn't init glew.");
		glfwTerminate();
		return -1;
	}

	CurrentAppRunning = this;
	glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *win, int width, int height) {
		CurrentAppRunning->OnResize(width, height);
	});

	ImGui_ImplGlfwGL3_Init(mWindow, true);
	OnStart();

	double lastTime = glfwGetTime();

	while (!glfwWindowShouldClose(mWindow)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		ImGui_ImplGlfwGL3_NewFrame();
		glfwPollEvents();

		auto currentTime = glfwGetTime();
		Update(currentTime - lastTime);

		Render();

		EQ::EventLoop::Get().Process();

		lastTime = currentTime;
	}

	OnShutdown();
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
}

void EQ::Graphics::App::OnStart() {
	
}

void EQ::Graphics::App::Update(double timeSinceLastFrame) {
}

void EQ::Graphics::App::Render() {
	ImGui::Render();
	glfwSwapBuffers(mWindow);
}

void EQ::Graphics::App::OnShutdown() {
	
}

void EQ::Graphics::App::OnResize(int width, int height) {
	glViewport(0, 0, width, height);
}