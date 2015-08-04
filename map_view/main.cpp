#include "zone.h"
#include "imgui_gflw.h"

int main(int argc, char **argv)
{
	eqLogInit(EQEMU_LOG_LEVEL);
	eqLogRegister(std::shared_ptr<EQEmu::Log::LogBase>(new EQEmu::Log::LogStdOut()));

	if(!glfwInit()) {
		eqLogMessage(LogFatal, "Couldn't init graphical system.");
		return -1;
	}

	std::string filename = "tutorialb";
	if(argc >= 2) {
		filename = argv[1];
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
#ifndef EQEMU_GL_DEP
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, 0);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_RESIZABLE, 0);
#endif

	GLFWwindow *win = glfwCreateWindow(RES_X, RES_Y, "Map View", nullptr, nullptr);
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

	glfwSetInputMode(win, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(win, GLFW_STICKY_KEYS, 1);
	glfwSetCursorPos(win, RES_X / 2, RES_Y / 2);

	std::unique_ptr<Zone> zone(new Zone(filename));
	zone->Load();

	ImGui_ImplGlfwGL3_Init(win, true);

	bool rendering = true;
	bool r_c = true;
	bool r_nc = true;
	bool r_vol = true;
	bool r_nav = true;
	char zone_input[256];
	strcpy(zone_input, filename.c_str());

	glCullFace(GL_BACK);

	do {
		auto &io = ImGui::GetIO();
		zone->UpdateInputs(win, io.WantCaptureKeyboard, io.WantCaptureMouse);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		if(glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(win) != 0)
			rendering = false;

		{
			ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Options");
			ImGui::InputText("Zone", zone_input, 256);
			if(ImGui::Button("Load Zone")) {
				zone.reset(new Zone(zone_input));
				zone->Load();
			}
			ImGui::Checkbox("Render Collidable Polygons", &r_c);
			ImGui::Checkbox("Render Non-Collidable Polygons", &r_nc);
			ImGui::Checkbox("Render Loaded Volumes", &r_vol);
			ImGui::Checkbox("Render Navigation", &r_nav);
			ImGui::End();
		}
		

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		
		glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		zone->Render(r_c, r_nc, r_vol, r_nav);

		ImGui::Render();

		glfwSwapBuffers(win);
	} while (rendering);

	glfwTerminate();
	return 0;
}
