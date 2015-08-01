#ifndef EQEMU_CAMERA_H
#define EQEMU_CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera(unsigned int width, unsigned int height, float fov, float near_clip, float far_clip);
	~Camera();

	void UpdateInputs(GLFWwindow *win, bool keyboard_in_use, bool mouse_in_use);
	glm::mat4 GetViewMat() { return view; }
	glm::mat4 GetProjMat() { return proj; }
	glm::vec3 GetLoc();
private:
	glm::vec3 pos;
	float hor_angle;
	float ver_angle;
	float fov;
	unsigned int height;
	unsigned int width;
	float near_clip;
	float far_clip;
	double last_time;
	bool first_input;
	glm::mat4 view;
	glm::mat4 proj;
};

#endif
