#include "application.h"
#include "imgui_glfw.h"
#include <gtc/matrix_transform.hpp>

std::string ReadFileToString(const std::string &filename) {
	auto f = fopen(filename.c_str(), "rb");

	if (f) {
		fseek(f, 0, SEEK_END);
		auto sz = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::string ret;
		ret.resize(sz);

		auto r = fread(&ret[0], sz, 1, f);
		if (r != 1) {
			fclose(f);
			return "";
		}

		fclose(f);
		return ret;
	}

	return "";
}

Application::Application()
{
	_width = 0;
	_height = 0;
	_show_open = false;
	_show_options = false;
	_show_debug = false;
	_camera_view = glm::mat4(0.0);
	_camera_proj = glm::mat4(0.0);
	_camera_loc = glm::vec3(0.0);
	_hor_angle = 3.14f;
	_ver_angle = 0.0f;
	_fov = 45.0f;
	_near_clip = 0.1f;
	_far_clip = 15000.0f;
	_last_time = glfwGetTime();
	_right_was_down = false;
}

Application::~Application()
{
}

void Application::Init(GLFWwindow *win)
{
	_window = win;
	glfwGetFramebufferSize(win, &_width, &_height);

	auto vert = ReadFileToString("phy.vert");
	auto frag = ReadFileToString("phy.frag");
	_shader.reset(new ShaderProgram(vert, frag));

	_model = _shader->GetUniformLocation("Model");
	_view = _shader->GetUniformLocation("View");
	_proj = _shader->GetUniformLocation("Proj");

	_actor.reset(new Actor());

	auto &verts = _actor->GetVerts();
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f));
	verts.push_back(ActorVertex(0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f));
	verts.push_back(ActorVertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f));

	auto &inds = _actor->GetIndices();
	for (size_t i = 0; i < verts.size(); ++i) {
		inds.push_back(static_cast<unsigned int>(i));
	}

	_actor->Compile();
}

void Application::Tick()
{
	glfwPollEvents();
	ProcessApplicationInput();
}

void Application::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDisable(GL_CULL_FACE);

	_shader->Use();
	
	glm::mat4 model = glm::mat4(1.0);
	
	_model.SetValueMatrix4(1, false, &model[0][0]);
	_view.SetValueMatrix4(1, false, &_camera_view[0][0]);
	_proj.SetValueMatrix4(1, false, &_camera_proj[0][0]);
	_actor->Draw();

	ImGui_ImplGlfwGL3_NewFrame();
	
	RenderMenu();
	RenderUI();

	ImGui::Render();
	glfwSwapBuffers(_window);
}

void Application::Resize(int width, int height)
{
	_width = width;
	_height = height;
	glViewport(0, 0, _width, _height);
}

void Application::RenderMenu()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("Open", "Ctrl+O", &_show_open);
		ImGui::MenuItem("Show Options", "Alt+O", &_show_options);
		ImGui::MenuItem("Show Debug", "Alt+G", &_show_debug);
		ImGui::Separator();
		if (ImGui::MenuItem("Quit", "Alt+Q")) { glfwSetWindowShouldClose(_window, 1); }
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void Application::RenderUI()
{
	if (_show_debug) {
		ImGui::Begin("Debug");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Loc: (%.2f, %.2f, %.2f)", _camera_loc.x, _camera_loc.y, _camera_loc.z);
		//ImGui::Text("Best floor: %.2f", m_physics->FindBestFloor(m_camera_loc, nullptr, nullptr));
		//ImGui::Text("Area: %s", GetRegionTypeString(m_physics->ReturnRegionType(m_camera_loc)));
		//if (GetZoneGeometry()) {
		//	auto zone_geo = GetZoneGeometry();
		//	ImGui::Text("Min: (%.2f, %.2f, %.2f)", zone_geo->GetCollidableMin().x, zone_geo->GetCollidableMin().y, zone_geo->GetCollidableMin().z);
		//	ImGui::Text("Max: (%.2f, %.2f, %.2f)", zone_geo->GetCollidableMax().x, zone_geo->GetCollidableMax().y, zone_geo->GetCollidableMax().z);
		//}
		ImGui::End();
	}

	if (_show_open) {
		ImGui::OpenPopup("Open Zone");
		_show_open = false;
	}

	if (ImGui::BeginPopupModal("Open Zone", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		bool use_popup = true;
		if (use_popup) {
			static char zone_name[256] = { 0 };
			if (ImGui::InputText("Zone Name", zone_name, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
			{
				ImGui::CloseCurrentPopup();
				//LoadScene(zone_name);
				strcpy(zone_name, "");
			}

			if ((glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
				strcpy(zone_name, "");

				ImGui::CloseCurrentPopup();
			}

			if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
				ImGui::SetKeyboardFocusHere(-1);

			if (ImGui::Button("OK", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				//LoadScene(zone_name);
				strcpy(zone_name, "");
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				strcpy(zone_name, "");
			}
		}
		ImGui::EndPopup();
	}
}

void Application::ProcessApplicationInput()
{
	if (_width == 0 || _height == 0) {
		return;
	}

	auto &io = ImGui::GetIO();
	auto t = glfwGetTime();
	double delta_time = _last_time - t;

	if (!io.WantCaptureMouse && glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (_right_was_down) {
			double x_pos, y_pos;
			int w, h;
			int display_w, display_h;
			glfwGetWindowSize(_window, &w, &h);
			glfwGetFramebufferSize(_window, &display_w, &display_h);

			glfwGetCursorPos(_window, &x_pos, &y_pos);
			x_pos *= (double)display_w / w;
			y_pos *= (double)display_h / h;

			glfwSetCursorPos(_window, _width / 2, _height / 2);

			_hor_angle += 0.005f * float(_width / 2 - x_pos);
			_ver_angle += 0.005f * float(_height / 2 - y_pos);
		}
		else {
			glfwSetCursorPos(_window, _width / 2, _height / 2);
		}
	}

	if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		_right_was_down = true;
	}
	else {
		_right_was_down = false;
	}

	glm::vec3 direction(cos(_ver_angle) * sin(_hor_angle), sin(_ver_angle), cos(_ver_angle) * cos(_hor_angle));
	glm::vec3 right = glm::vec3(sin(_hor_angle - 3.14f / 2.0f), 0, cos(_hor_angle - 3.14f / 2.0f));
	glm::vec3 up = glm::cross(right, direction);

	float speed = 10.0f;
	bool move = true;

	if (!io.WantCaptureKeyboard && (
		glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)) {
		move = false;
	}

	if (!io.WantCaptureKeyboard && (
		glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
		glfwGetKey(_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		speed *= 6.0f;
	}

	if (!io.WantCaptureKeyboard && move && glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS) {
		_camera_loc += direction * static_cast<float>(delta_time) * speed;
	}

	if (!io.WantCaptureKeyboard && move && glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS) {
		_camera_loc -= direction * static_cast<float>(delta_time) * speed;
	}

	if (!io.WantCaptureKeyboard && move && glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS) {
		_camera_loc += right * static_cast<float>(delta_time) * speed;
	}

	if (!io.WantCaptureKeyboard && move && glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS) {
		_camera_loc -= right * static_cast<float>(delta_time) * speed;
	}

	_camera_proj = glm::perspective(_fov, (float)_width / (float)_height, _near_clip, _far_clip);
	_camera_view = glm::lookAt(_camera_loc, _camera_loc + direction, up);

	_last_time = t;
}

void Application::LoadScene(const std::string &zone_name)
{
	//Clear existing scene

	//Reset camera pos

	//Load new scene
}
