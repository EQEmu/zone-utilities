#include "scene.h"
#include "imgui_glfw.h"
#include <gtc/matrix_transform.hpp>

#include "static_geometry.h"

Scene::Scene() {
}

Scene::~Scene() {
	UnregisterAllModules();
}

enum MainHotkeys : int
{
	MainHotkeyQuit,
	MainHotkeyOpen,
	MainHotkeySave,
	MainHotkeyToggleOptions,
	MainHotkeyToggleDebug,
};

void Scene::Init(GLFWwindow *win, int width, int height) {
	m_window = win;
	m_width = width;
	m_height = height;
	m_name[0] = 0;

	m_hor_angle = 3.14f;
	m_ver_angle = 0.0f;
	m_fov = 45.0f;
	m_near_clip = 0.1f;
	m_far_clip = 15000.0f;
	m_last_time = 0.0f;
	m_first_input = true;

	for(int i = 0; i < 512; ++i) {
		m_key_status[i] = GLFW_RELEASE;
	}

	m_show_open = false;
	m_show_save = false;
	m_show_options = false;
	m_show_debug = false;
	m_render_collide = true;
	m_render_non_collide = true;
	m_render_volume = true;

	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);
	glViewport(0, 0, width, height);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	glfwSetInputMode(win, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(win, GLFW_STICKY_KEYS, 1);
	ImGui_ImplGlfwGL3_Init(win, true);

	RegisterHotkey(this, MainHotkeyQuit, GLFW_KEY_Q, false, true, false);
	RegisterHotkey(this, MainHotkeyOpen, GLFW_KEY_O, true, false, false);
	RegisterHotkey(this, MainHotkeySave, GLFW_KEY_S, true, false, false);
	RegisterHotkey(this, MainHotkeyToggleOptions, GLFW_KEY_O, false, true, false);
	RegisterHotkey(this, MainHotkeyToggleDebug, GLFW_KEY_G, false, true, false);

	m_physics.reset(new EQPhysics());
#ifdef EQEMU_GL_DEP
	m_shader.reset(new ShaderProgram("shaders/basic.vert", "shaders/basic.frag"));
#else
	m_shader.reset(new ShaderProgram("shaders/basic130.vert", "shaders/basic130.frag"));
#endif

	m_mvp = m_shader->GetUniformLocation("MVP");
	m_tint = m_shader->GetUniformLocation("Tint");
}

void Scene::LoadScene(const char *zone_name) {
	m_name = zone_name;
	m_hor_angle = 3.14f;
	m_ver_angle = 0.0f;
	m_camera_loc.x = 0.0f;
	m_camera_loc.y = 0.0f;
	m_camera_loc.z = 0.0f;

	m_zone_geometry.reset(ZoneMap::LoadMapFile(zone_name));
	if(m_zone_geometry) {
		m_physics.reset(new EQPhysics());

		WaterMap *w_map = WaterMap::LoadWaterMapfile(zone_name);
		m_physics->SetCollidableWorld(m_zone_geometry->GetCollidableVerts(), m_zone_geometry->GetCollidableInds());
		m_physics->SetNonCollidableWorld(m_zone_geometry->GetNonCollidableVerts(), m_zone_geometry->GetNonCollidableInds());
		m_physics->SetWaterMap(w_map);

		//create models from the loaded stuff here...
		StaticGeometry *m = new StaticGeometry();
		m->GetVerts() = m_zone_geometry->GetCollidableVerts();
		m->GetInds() = m_zone_geometry->GetCollidableInds();
		m->SetTint(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
		m->Compile();
		m_collide_mesh_entity.reset(m);

		m = new StaticGeometry();
		m->GetVerts() = m_zone_geometry->GetNonCollidableVerts();
		m->GetInds() = m_zone_geometry->GetNonCollidableInds();
		m->SetTint(glm::vec4(0.5f, 0.7f, 1.0f, 1.0f));
		m->Compile();
		m_non_collide_mesh_entity.reset(m);

		m = new StaticGeometry();
		if(w_map) {
			w_map->CreateMeshFrom(m->GetVerts(), m->GetInds());
		}
		m->SetTint(glm::vec4(0.0f, 0.0f, 0.8f, 0.2f));
		m->Compile();
		m_volume_mesh_entity.reset(m);
	}

	for(auto &module : m_modules) {
		module->OnSceneLoad(m_name.c_str());
	}
}

void Scene::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	ImGui_ImplGlfwGL3_NewFrame();

	m_shader->Use();

	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 mvp = m_camera_proj * m_camera_view * model;
	m_mvp.SetValueMatrix4(1, false, &mvp[0][0]);

	//render all our models here
	if(m_render_collide && m_collide_mesh_entity) {
		glm::vec4 tint = m_collide_mesh_entity->GetTint();
		m_tint.SetValuePtr4(1, &tint[0]);
		m_collide_mesh_entity->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		tint.x = 0.0f;
		tint.y = 0.0f;
		tint.z = 0.0f;
		tint.w = 0.0f;
		m_tint.SetValuePtr4(1, &tint[0]);
		m_collide_mesh_entity->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if(m_render_non_collide && m_non_collide_mesh_entity) {
		glm::vec4 tint = m_non_collide_mesh_entity->GetTint();
		m_tint.SetValuePtr4(1, &tint[0]);
		m_non_collide_mesh_entity->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		tint.x = 0.0f;
		tint.y = 0.0f;
		tint.z = 0.0f;
		tint.w = 0.0f;
		m_tint.SetValuePtr4(1, &tint[0]);
		m_non_collide_mesh_entity->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if(m_render_volume && m_volume_mesh_entity) {
		glEnable(GL_BLEND);
		glm::vec4 tint = m_volume_mesh_entity->GetTint();
		m_tint.SetValuePtr4(1, &tint[0]);
		m_volume_mesh_entity->Draw();
		glDisable(GL_BLEND);
	}

	for(auto &e : m_registered_entities) {
		glm::vec4 tint = e->GetTint();
		m_tint.SetValuePtr4(1, &tint[0]);

		auto &pos = e->GetLocation();
		model[3][0] = pos.x;
		model[3][1] = pos.y;
		model[3][2] = pos.z;
		mvp = m_camera_proj * m_camera_view * model;
		m_mvp.SetValueMatrix4(1, false, &mvp[0][0]);
		e->Draw();
	}

	//render our main menu
	ImGui::BeginMainMenuBar();
	RenderMainMenu();
	RenderModulesMenu();
	ImGui::EndMainMenuBar();

	RenderUI();

	//render our modules ui stuff
	for(auto &module : m_modules) {
		if(module->GetRunning() && module->GetUnpaused()) {
			module->OnDrawUI();
		}
	}

	ImGui::Render();
	glfwSwapBuffers(m_window);
}

void Scene::RenderMainMenu() {
	if(ImGui::BeginMenu("File"))
	{
		ImGui::MenuItem("Open", "Ctrl+O", &m_show_open);
		ImGui::MenuItem("Save", "Ctrl+S", &m_show_save);
		ImGui::Separator();
		ImGui::MenuItem("Show Options", "Alt+O", &m_show_options);
		ImGui::MenuItem("Show Debug", "Alt+G", &m_show_debug);
		ImGui::Separator();
		if(ImGui::MenuItem("Quit", "Alt+Q")) { glfwSetWindowShouldClose(m_window, 1); }
		ImGui::EndMenu();
	}
}

void Scene::RenderUI() {
	if(m_show_debug) {
		ImGui::Begin("Debug");
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Loc: (%.2f, %.2f, %.2f)", m_camera_loc.x, m_camera_loc.y, m_camera_loc.z);
		ImGui::Text("Best floor: %.2f", m_physics->FindBestFloor(m_camera_loc, nullptr, nullptr));
		ImGui::Text("InLiquid: %s", m_physics->InLiquid(m_camera_loc) ? "true" : "false");
		ImGui::End();
	}
	
	if(m_show_options) {
		ImGui::Begin("Options");
		ImGui::Checkbox("Render Collidable Mesh", &m_render_collide);
		ImGui::Checkbox("Render Non-Collidable Mesh", &m_render_non_collide);
		ImGui::Checkbox("Render Volumes", &m_render_volume);
		ImGui::End();
	}
	
	for(auto &module : m_modules) {
		if(module->HasWork()) {
			m_show_open = false;
			m_show_save = false;
		}
	}

	if(m_show_open) {
		ImGui::OpenPopup("Open Zone");
		m_show_open = false;
	}

	if(m_show_save) {
		//ImGui::OpenPopup("Save Data");
		m_show_save = false;
	}
	
	if(ImGui::BeginPopupModal("Open Zone", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		bool use_popup = true;
		for(auto &module : m_modules) {
			if(module->HasWork()) {
				ImGui::CloseCurrentPopup();
				use_popup = false;
				break;
			}
		}
		if(use_popup) {
			static char zone_name[256];
			if(ImGui::InputText("Zone Name", zone_name, 256, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
			{
				ImGui::CloseCurrentPopup();
				LoadScene(zone_name);
				strcpy(zone_name, "");
			}
	
			if((glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)) {
				strcpy(zone_name, "");
	
				ImGui::CloseCurrentPopup();
			}
	
			if(ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
				ImGui::SetKeyboardFocusHere(-1);
	
			if(ImGui::Button("OK", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				LoadScene(zone_name);
				strcpy(zone_name, "");
			}
			ImGui::SameLine();
			if(ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				strcpy(zone_name, "");
			}
		}
		ImGui::EndPopup();
	}
}

void Scene::RenderModulesMenu() {
	if(ImGui::BeginMenu("Modules"))
	{
		for(auto &module : m_modules) {
			if(!module->GetRunning()) {
				ImGui::MenuItem(module->GetName(), nullptr, &module->GetRunning());
			} else {
				if(ImGui::MenuItem(module->GetName())) {
					if(module->GetUnpaused()) {
						module->OnSuspend();
					} else {
						module->OnResume();
					}

					module->SetUnpaused(!module->GetUnpaused());
				}
			}
		}
		ImGui::EndMenu();
	}

	//go through and render all active modules...
	for(auto &module : m_modules) {
		if(module->GetRunning() && module->GetUnpaused()) {
			module->OnDrawMenu();
		}
	}
}

void Scene::Tick() {
	glfwPollEvents();

	if(!TryHotkey()) 
		ProcessCamera();
}

void Scene::ProcessCamera() {
	auto &io = ImGui::GetIO();

	if(m_first_input) {
		m_last_time = glfwGetTime();
		m_first_input = false;
	}

	double current_time = glfwGetTime();
	float delta_time = float(current_time - m_last_time);

	if(!io.WantCaptureMouse && glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		double x_pos, y_pos;
		glfwGetCursorPos(m_window, &x_pos, &y_pos);
		glfwSetCursorPos(m_window, m_width / 2, m_height / 2);

		m_hor_angle += 0.005f * float(m_width / 2 - x_pos);
		m_ver_angle += 0.005f * float(m_height / 2 - y_pos);
	}

	glm::vec3 direction(cos(m_ver_angle) * sin(m_hor_angle), sin(m_ver_angle), cos(m_ver_angle) * cos(m_hor_angle));
	glm::vec3 right = glm::vec3(sin(m_hor_angle - 3.14f / 2.0f), 0, cos(m_hor_angle - 3.14f / 2.0f));
	glm::vec3 up = glm::cross(right, direction);

	float speed = 50.0f;

	if(!io.WantCaptureKeyboard && (
		glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
		glfwGetKey(m_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)) {
		speed *= 6.0f;
	}

	if(!io.WantCaptureKeyboard && glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS){
		m_camera_loc += direction * delta_time * speed;
	}

	if(!io.WantCaptureKeyboard && glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS){
		m_camera_loc -= direction * delta_time * speed;
	}

	if(!io.WantCaptureKeyboard && glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS){
		m_camera_loc += right * delta_time * speed;
	}

	if(!io.WantCaptureKeyboard && glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS){
		m_camera_loc -= right * delta_time * speed;
	}

	m_camera_proj = glm::perspective(m_fov, (float)m_width / (float)m_height, m_near_clip, m_far_clip);
	m_camera_view = glm::lookAt(m_camera_loc, m_camera_loc + direction, up);

	m_last_time = current_time;
}

bool Scene::TryHotkey() {
	//ImGui takes over the glfw callbacks unfortunately
	//so piggy back it for keydown events.
	auto &io = ImGui::GetIO();
	if(io.WantCaptureKeyboard) {
		return false;
	}

	bool hotkey_hit = false;
	int mods = 0;

	if(io.KeyShift) {
		mods |= GLFW_MOD_SHIFT;
	}

	if(io.KeyCtrl) {
		mods |= GLFW_MOD_CONTROL;
	}

	if(io.KeyAlt) {
		mods |= GLFW_MOD_ALT;
	}

	for(auto &hotkey : m_hotkeys) {
		if(!hotkey_hit && io.KeysDown[hotkey.key] == GLFW_RELEASE && m_key_status[hotkey.key] == GLFW_PRESS && hotkey.modifiers & mods) {
			hotkey.system->OnHotkey(hotkey.id);
			hotkey_hit = true;
		}
	}

	for(auto &hotkey : m_hotkeys) {
		m_key_status[hotkey.key] = io.KeysDown[hotkey.key];
	}

	return hotkey_hit;
}

void Scene::RegisterHotkey(SceneHotkeyListener *system, int ident, int key, bool ctrl, bool alt, bool shift) {
	if(!system) {
		return;
	}

	HotkeyEntry entry;
	entry.id = ident;
	entry.key = key;
	entry.modifiers = 0;

	if(shift)
		entry.modifiers |= GLFW_MOD_SHIFT;

	if(ctrl)
		entry.modifiers |= GLFW_MOD_CONTROL;

	if(alt)
		entry.modifiers |= GLFW_MOD_ALT;

	entry.system = system;

	m_hotkeys.push_back(entry);
}

void Scene::UnregisterHotkey(SceneHotkeyListener *system, int ident) {
	for(auto iter = m_hotkeys.begin(); iter != m_hotkeys.end(); ++iter) {
		if(iter->system == system && iter->id == ident) {
			m_hotkeys.erase(iter);
			return;
		}
	}
}

void Scene::OnHotkey(int ident) {
	switch(ident) {
		case MainHotkeyQuit:
			glfwSetWindowShouldClose(m_window, 1);
			break;
		case MainHotkeyOpen:
			m_show_open = true;
			break;
		case MainHotkeySave:
			m_show_save = true;
			break;
		case MainHotkeyToggleOptions:
			m_show_options = !m_show_options;
			break;
		case MainHotkeyToggleDebug:
			m_show_debug = !m_show_debug;
			break;
	}
}

void Scene::RegisterEntity(Entity *e) {
	UnregisterEntity(e);
	m_registered_entities.push_back(e);
}

void Scene::UnregisterEntity(Entity *e) {
	for(auto iter = m_registered_entities.begin(); iter != m_registered_entities.end(); ++iter) {
		if((*iter) == e) {
			m_registered_entities.erase(iter);
			return;
		}
	}
}

void Scene::RegisterModule(Module *m) {
	m->OnLoad(this);
	m_modules.push_back(std::unique_ptr<Module>(m));
}

void Scene::UnregisterModule(Module *m) {
	auto iter = m_modules.begin();
	while(iter != m_modules.end()) {
		if(m == iter->get()) {
			m->OnShutdown();
			m_modules.erase(iter);
			return;
		}

		++iter;
	}
}

void Scene::UnregisterAllModules() {
	for(auto &module : m_modules) {
		module->OnShutdown();
	}

	m_modules.clear();
}
