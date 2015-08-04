#include "zone.h"

Zone::Zone(const std::string &name)
:
m_name(name),
m_camera(RES_X, RES_Y, 45.0f, 0.1f, 15000.0f),
#ifndef EQEMU_GL_DEP
m_shader("shaders/basic.vert", "shaders/basic.frag")
#else
m_shader("shaders/basic130.vert", "shaders/basic130.frag")
#endif
{
	m_render_debug = true;
}

void Zone::Load()
{
	m_uniform = m_shader.GetUniformLocation("MVP");
	m_tint = m_shader.GetUniformLocation("Tint");
	Model *collide = nullptr;
	Model *invis = nullptr;
	Model *volume = nullptr;

	LoadMap(m_name, &collide, &invis);
	LoadWaterMap(m_name, &volume);

	if(collide == nullptr)
		eqLogMessage(LogWarn, "Couldn't load zone geometry from %s.map.", m_name.c_str());

	if(volume == nullptr)
		eqLogMessage(LogWarn, "Couldn't load zone volumes from %s.wtr, either does not exist or is a version 1 wtr file which can't be displayed.", m_name.c_str());

	m_collide.reset(collide);
	m_invis.reset(invis);
	m_volume.reset(volume);

	if(collide) {
		z_map = std::unique_ptr<ZoneMap>(ZoneMap::LoadMapFromData(collide->GetPositions(), collide->GetIndicies()));
	} else {
		z_map = std::unique_ptr<ZoneMap>(ZoneMap::LoadMapFile(m_name));
	}
	w_map = std::unique_ptr<WaterMap>(WaterMap::LoadWaterMapfile(m_name));

	if(z_map && w_map && m_collide)
		m_nav = std::unique_ptr<Navigation>(new Navigation(z_map.get(), w_map.get(), m_collide.get(), m_camera));

	prev_history_point = std::chrono::system_clock::now();
}

void Zone::Render(bool r_c, bool r_nc, bool r_vol, bool r_nav) {
	auto now = std::chrono::system_clock::now();
	if(std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_history_point).count() >= 100) {
		if(m_render_frame_rate_history.size() > 1200) {
			m_render_frame_rate_history.erase(m_render_frame_rate_history.begin(), m_render_frame_rate_history.begin() + 200);
		}

		m_render_frame_rate_history.push_back(ImGui::GetIO().Framerate);
		prev_history_point = now;
	}

	if(m_render_debug) { RenderDebugWindow(); }

	if(m_nav) {
		m_nav->RenderGUI();
	}

	glEnable(GL_CULL_FACE);

	glDisable(GL_BLEND);
	m_shader.Use();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 proj = m_camera.GetProjMat();
	glm::mat4 view = m_camera.GetViewMat();
	glm::mat4 mvp = proj * view * model;
	m_uniform.SetValueMatrix4(1, false, &mvp[0][0]);

	glm::vec4 tnt(0.8f, 0.8f, 0.8f, 1.0f);
	m_tint.SetValuePtr4(1, &tnt[0]);

	if(m_collide && r_c)
		m_collide->Draw();

	tnt[0] = 0.5f;
	tnt[1] = 0.7f;
	tnt[2] = 1.0f;
	m_tint.SetValuePtr4(1, &tnt[0]);

	if(m_invis && r_nc)
		m_invis->Draw();

	if(r_nav && m_nav) {
		m_nav->Draw();
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	tnt[0] = 0.0f;
	tnt[1] = 0.0f;
	tnt[2] = 0.8f;
	tnt[3] = 0.2f;
	m_tint.SetValuePtr4(1, &tnt[0]);

	glDisable(GL_CULL_FACE);
	if(m_volume && r_vol)
		m_volume->Draw();

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	tnt[0] = 0.0f;
	tnt[1] = 0.0f;
	tnt[2] = 0.0f;
	tnt[3] = 0.0f;
	m_tint.SetValuePtr4(1, &tnt[0]);

	if(m_collide && r_c)
		m_collide->Draw();

	if(m_invis && r_nc)
		m_invis->Draw();

	if(m_volume && r_vol)
		m_volume->Draw();

	if(m_nav && r_nav) {
		m_nav->Draw();
		glDisable(GL_DEPTH_TEST);
		m_nav->DrawSelectionConnections();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		m_nav->DrawSelection();
		glEnable(GL_DEPTH_TEST);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Zone::UpdateInputs(GLFWwindow *win, bool keyboard_in_use, bool mouse_in_use) {
	if(m_nav && !mouse_in_use && glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		double x_pos, y_pos;
		glfwGetCursorPos(win, &x_pos, &y_pos);
		m_nav->RaySelection((int)x_pos, RES_Y - (int)y_pos);
	}

	m_camera.UpdateInputs(win, keyboard_in_use, mouse_in_use);
}

void Zone::RenderDebugWindow() {
	glm::vec3 loc = m_camera.GetLoc();
	glm::vec3 min;
	glm::vec3 max;
	if(m_collide) {
		min = m_collide->GetAABBMin();
	}

	if(z_map) {
		max = m_collide->GetAABBMax();
	}

	ImGui::SetNextWindowSize(ImVec2(640, 120), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Debug Stats", NULL, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::TextColored(ImVec4(0.3, 1.0, 0.3, 1.0), "W S A D to Move, Hold RMB to rotate. Hold Shift to speed boost.");

	if(m_render_frame_rate_history.size() > 0) {
		ImGui::PlotLines("", &m_render_frame_rate_history[0], (int)m_render_frame_rate_history.size(), 0, NULL, 0.0f, 60.0f);
		ImGui::SameLine();
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	} else {
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	ImGui::Text("Zone: %s, min: (%.2f, %.2f, %.2f), max: (%.2f, %.2f, %.2f)", m_name.c_str(), min.x, min.z, min.y, max.x, max.z, max.y);
	ImGui::Text("EQ Coords: %.2f, %.2f, %.2f", loc.x, loc.z, loc.y);
	if(z_map && w_map) {
		ImGui::Text("Floor: %.2f, In Liquid: %s", z_map->FindBestFloor(loc, nullptr, nullptr),
					w_map->InLiquid(loc.x, loc.z, loc.y) ? "true" : "false");
	}
	else if(z_map) {
		ImGui::Text("Floor: %.2f", z_map->FindBestFloor(loc, nullptr, nullptr));
	}
	else if(w_map) {
		ImGui::Text("In Liquid: %s", w_map->InLiquid(loc.x, loc.z, loc.y) ? "true" : "false");
	}
	ImGui::End();
}
