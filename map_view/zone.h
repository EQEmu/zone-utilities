#ifndef EQEMU_MAPVIEW_ZONE_H
#define EQEMU_MAPVIEW_ZONE_H

#include <memory>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "map.h"
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "zone_map.h"
#include "water_map.h"
#include "log_macros.h"
#include "log_stdout.h"
#include "imgui.h"

#define RES_X 1600
#define RES_Y 900

class Zone
{
public:
	Zone(const std::string &name);
	~Zone() { }

	void Load();
	void Render(bool r_c, bool r_nc, bool r_vol);
	void UpdateInputs(GLFWwindow *win, bool keyboard_inuse, bool mouse_inuse) { m_camera.UpdateInputs(win, keyboard_inuse, mouse_inuse); }

private:
	std::string m_name;
	ShaderProgram m_shader;
	ShaderUniform m_uniform;
	ShaderUniform m_tint;
	Camera m_camera;

	std::unique_ptr<Model> m_collide;
	std::unique_ptr<Model> m_invis;
	std::unique_ptr<Model> m_volume;
	std::unique_ptr<ZoneMap> z_map;
	std::unique_ptr<WaterMap> w_map;

};


#endif
