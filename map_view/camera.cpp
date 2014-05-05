#include "camera.h"

Camera::Camera(unsigned int width, unsigned int height, float fov, float near_clip, float far_clip) {
	hor_angle = 3.14f;
	ver_angle = 0.0f;
	this->fov = fov;
	this->height = height;
	this->width = width;
	this->near_clip = near_clip;
	this->far_clip = far_clip;
	last_time = 0.0;
	first_input = true;
	pos = glm::vec3(0, 0, 5);
}

Camera::~Camera() {
}

void Camera::UpdateInputs(GLFWwindow *win) {
	if(first_input) {
		last_time = glfwGetTime();
		first_input = false;
	}

	double current_time = glfwGetTime();
	float delta_time = float(current_time - last_time);

	double x_pos, y_pos;
	glfwGetCursorPos(win, &x_pos, &y_pos);

	glfwSetCursorPos(win, width / 2, height / 2);

	hor_angle += 0.005f * float(width / 2 - x_pos);
	ver_angle += 0.005f * float(height / 2 - y_pos);

	glm::vec3 direction(cos(ver_angle) * sin(hor_angle), sin(ver_angle),	cos(ver_angle) * cos(hor_angle));
	glm::vec3 right = glm::vec3(sin(hor_angle - 3.14f / 2.0f), 0, cos(hor_angle - 3.14f / 2.0f));
	glm::vec3 up = glm::cross(right, direction);

	if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS){
		pos += direction * delta_time * 300.0f;
	}

	if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS){
		pos -= direction * delta_time * 300.0f;
	}
	
	if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS){
		pos += right * delta_time * 300.0f;
	}
	
	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS){
		pos -= right * delta_time * 300.0f;
	}

	proj = glm::perspective(fov, (float)width / (float)height, near_clip, far_clip);
	view = glm::lookAt(pos, pos + direction, up);

	last_time = current_time;
}
