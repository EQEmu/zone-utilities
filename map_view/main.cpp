#include "zone.h"
#include "imgui_gflw.h"

void RenderMainMenu(GLFWwindow *win, bool &render_options, bool &rendering, std::unique_ptr<Zone>& zone, bool &open_triggered);
void RenderOptionsWindow(bool &render_collide, bool &render_non_collide, bool &render_volumes, bool &render_navigation);

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
	bool r_opts = false;
	bool open_triggered = false;

	glCullFace(GL_BACK);

	do {
		auto &io = ImGui::GetIO();
		zone->UpdateInputs(win, io.WantCaptureKeyboard, io.WantCaptureMouse);
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		if(glfwWindowShouldClose(win) != 0)
			rendering = false;

		if((glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) && glfwGetKey(win, GLFW_KEY_O)) {
			open_triggered = true;
		}

		RenderMainMenu(win, r_opts, rendering, zone, open_triggered);
		if(r_opts) { RenderOptionsWindow(r_c, r_nc, r_vol, r_nav); }
		
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

void RenderMainMenu(GLFWwindow *win, bool &render_options, bool &rendering, std::unique_ptr<Zone>& zone, bool &open_triggered) {
	if(open_triggered) {
		ImGui::OpenPopup("Open File");
		open_triggered = false;
	}

	if(ImGui::BeginPopupModal("Open File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char zone_name[256];
		if(ImGui::InputText("Zone:", zone_name, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
		{
			ImGui::CloseCurrentPopup();

			zone.reset(new Zone(zone_name));
			zone->Load();
			strcpy(zone_name, "");
		}

		if((glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
			strcpy(zone_name, "");

			ImGui::CloseCurrentPopup();
		}

		if(ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
			ImGui::SetKeyboardFocusHere(-1);

		if(ImGui::Button("OK", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup(); 

			zone.reset(new Zone(zone_name));
			zone->Load();
			strcpy(zone_name, "");
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel", ImVec2(120, 0))) { 
			ImGui::CloseCurrentPopup();

			strcpy(zone_name, "");
		}
		ImGui::EndPopup();
	}

	ImGui::BeginMainMenuBar();
	if(ImGui::BeginMenu("File"))
	{
		if(ImGui::MenuItem("Open", "CTRL + O", &open_triggered)) { }
		if(ImGui::MenuItem("Save", NULL)) { }

		ImGui::Separator();
		ImGui::MenuItem("Show Options", NULL, &render_options);
		ImGui::MenuItem("Show Debug", NULL, &zone->GetRenderDebugWindow());
		ImGui::Separator();
		if(ImGui::MenuItem("Quit", "Alt+F4")) { rendering = false; }
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void RenderOptionsWindow(bool &render_collide, bool &render_non_collide, bool &render_volumes, bool &render_navigation) {
	ImGui::Begin("Options", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Once);
	if(ImGui::TreeNode("Rendering")) {
		ImGui::Checkbox("Render Collidable Polygons", &render_collide);
		ImGui::Checkbox("Render Non-Collidable Polygons", &render_non_collide);
		ImGui::Checkbox("Render Loaded Volumes", &render_volumes);
		ImGui::Checkbox("Render Navigation", &render_navigation);
	}
	ImGui::End();
}
