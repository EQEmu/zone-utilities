#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "map.h"
#include "log_macros.h"
#include "log_stdout.h"

int main(int argc, char **argv)
{
	eqLogInit(EQEMU_LOG_LEVEL);
	std::shared_ptr<EQEmu::Log::LogBase> stdout_log(new EQEmu::Log::LogStdOut());
	eqLogRegister(stdout_log);

	if(!glfwInit()) {
		return -1;
	}

	std::string filename = "tutorialb.map";
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

	GLFWwindow *win = glfwCreateWindow(1280, 720, "Map View", nullptr, nullptr);
	if(!win) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(win);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK) {
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(win, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(win, 1280 / 2, 720 / 2);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

#ifndef EQEMU_GL_DEP
	ShaderProgram shader("shaders/basic.vert", "shaders/basic.frag");
#else
	ShaderProgram shader("shaders/basic130.vert", "shaders/basic130.frag");
#endif
	ShaderUniform uniform = shader.GetUniformLocation("MVP");
	ShaderUniform tint = shader.GetUniformLocation("Tint");

	Model *collide = nullptr;
	Model *invis = nullptr;
	LoadMap(filename, &collide, &invis);

	Camera cam(1280, 720, 45.0f, 0.1f, 15000.0f);

	bool rendering = true;
	bool r_nc_pressed = false;
	bool r_nc = true;
	do {
		double current_frame_time = glfwGetTime();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		cam.UpdateInputs(win);

		shader.Use();

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		glm::mat4 model = glm::mat4(1.0);
		glm::mat4 mvp = cam.GetProjMat() * cam.GetViewMat() * model;
		uniform.SetValueMatrix4(1, false, &mvp[0][0]);

		glm::vec3 tnt(0.8f, 0.8f, 0.8f);
		tint.SetValuePtr3(1, &tnt[0]);

		if (collide)
			collide->Draw();
		
		tnt[0] = 0.5f;
		tnt[1] = 0.7f;
		tnt[2] = 1.0f;
		tint.SetValuePtr3(1, &tnt[0]);

		if (invis && r_nc)
			invis->Draw();

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		tnt[0] = 0.0f;
		tnt[1] = 0.0f;
		tnt[2] = 0.0f;
		tint.SetValuePtr3(1, &tnt[0]);

		if (collide)
			collide->Draw();

		if (invis && r_nc)
			invis->Draw();

		glfwSwapBuffers(win);
		glfwPollEvents();

		if (glfwGetKey(win, GLFW_KEY_N) == GLFW_PRESS) {
			r_nc_pressed = true;
		}

		if (glfwGetKey(win, GLFW_KEY_N) == GLFW_RELEASE && r_nc_pressed) {
			r_nc = !r_nc;
			r_nc_pressed = false;
		}

		if(glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(win) != 0)
			rendering = false;
	} while (rendering);

	if(collide)
		delete collide;

	if (invis)
		delete invis;

	glfwTerminate();
	return 0;
}
