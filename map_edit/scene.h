#ifndef EQEMU_MAP_EDIT_MAIN_SCENE_H
#define EQEMU_MAP_EDIT_MAIN_SCENE_H

#include <vector>
#include <string>
#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <log_macros.h>
#include <log_stdout.h>
#include <imgui.h>
#include <glm.hpp>

#include "module.h"
#include "eq_physics.h"
#include "zone_map.h"
#include "entity.h"
#include "shader.h"
#include "dynamic_geometry.h"

class SceneHotkeyListener
{
public:
	SceneHotkeyListener() { }
	virtual ~SceneHotkeyListener() { }

	virtual void OnHotkey(int ident) = 0;
};

struct HotkeyEntry
{
	int id;
	int key;
	int modifiers;
	SceneHotkeyListener *system;
};

class Scene : public SceneHotkeyListener
{
public:
	Scene();
	~Scene();
	
	//scene & engine managment
	void Init(GLFWwindow *win, int width, int height);
	void LoadScene(const char *zone_name);
	void Render();
	void RenderMainMenu();
	void RenderUI();
	void RenderModulesMenu();
	void Tick();
	void ProcessSceneInput();

	//hotkeys
	void RegisterHotkey(SceneHotkeyListener *system, int ident, int key, bool ctrl, bool alt, bool shift);
	void UnregisterHotkey(SceneHotkeyListener *system, int ident);
	virtual void OnHotkey(int ident);

	//entity
	void RegisterEntity(Module *m, Entity *e, bool selectable = false);
	void UnregisterEntity(Module *m, Entity *e);
	void UnregisterEntitiesByModule(Module *m);

	//module
	void RegisterModule(Module *m);
	void UnregisterModule(Module *m);
	void UnregisterAllModules();

	std::shared_ptr<ZoneMap> GetZoneGeometry() { return m_zone_geometry; }
	std::shared_ptr<EQPhysics> GetZonePhysics() { return m_physics; }
	std::string GetZoneName() { return m_name; }

	const glm::vec3 &GetCameraLoc() { return m_camera_loc; }
	glm::vec3 &GetBoundingBoxMin() { return m_bounding_box_min; }
	glm::vec3 &GetBoundingBoxMax() { return m_bounding_box_max; }
	void UpdateBoundingBox();
private:
	void GetEntityName(Entity *ent, std::string &name);
	void GetClickVectors(double x, double y, glm::vec3 &start, glm::vec3 &end);
	friend class Module;
	Scene(const Scene&);
	Scene& operator=(const Scene&);

	//graphics info
	GLFWwindow *m_window;
	int m_width;
	int m_height;

	//camera info
	glm::mat4 m_camera_view;
	glm::mat4 m_camera_proj;
	glm::vec3 m_camera_loc;
	float m_hor_angle;
	float m_ver_angle;
	float m_fov;
	float m_near_clip;
	float m_far_clip;
	double m_last_time;
	bool m_first_input;
	bool m_right_was_down;

	//zone info
	std::string m_name;
	std::shared_ptr<ZoneMap> m_zone_geometry;
	std::shared_ptr<EQPhysics> m_physics;

	//Entity / rendering
	std::unique_ptr<ShaderProgram> m_shader;
	ShaderUniform m_model;
	ShaderUniform m_view;
	ShaderUniform m_proj;
	ShaderUniform m_tint;
	std::unique_ptr<Entity> m_collide_mesh_entity;
	std::unique_ptr<Entity> m_non_collide_mesh_entity;
	std::map<Module*, std::vector<Entity*>> m_registered_entities;

	//bounds
	std::unique_ptr<DynamicGeometry> m_bounding_box_renderable;
	glm::vec3 m_bounding_box_min;
	glm::vec3 m_bounding_box_max;

	//hotkeys
	bool TryHotkey();
	std::vector<HotkeyEntry> m_hotkeys;
	int m_key_status[512];
	int m_mouse_status[5];

	//modules
	std::vector<std::unique_ptr<Module>> m_modules;

	//ui flags
	bool m_show_open;
	bool m_show_save;
	bool m_show_options;
	bool m_show_debug;
	bool m_render_collide;
	bool m_render_non_collide;
	bool m_render_bb;
};

#endif
