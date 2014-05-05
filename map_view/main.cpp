#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "shader.h"
#include "model.h"
#include "camera.h"
#include "map.h"

int main(int argc, char **argv)
{
	if(!glfwInit()) {
		return -1;
	}

	std::string filename = "tutorialb.map";
	if(argc >= 2) {
		filename = argv[1];
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *win = glfwCreateWindow(1280, 720, "Zone View", nullptr, nullptr);
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

	ShaderProgram shader("basic.vert", "basic.frag");
	ShaderUniform uniform = shader.GetUniformLocation("MVP");
	ShaderUniform tint = shader.GetUniformLocation("Tint");

	Model *collide = nullptr;
	Model *invis = nullptr;
	LoadMap(filename, &collide, &invis);

	Camera cam(1280, 720, 45.0f, 0.1f, 10000.0f);

	bool rendering = true;
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		cam.UpdateInputs(win);

		shader.Use();
		
		glm::mat4 model = glm::mat4(1.0);
		glm::mat4 mvp = cam.GetProjMat() * cam.GetViewMat() * model;
		uniform.SetValueMatrix4(1, false, &mvp[0][0]);
		
		glm::vec3 tnt(1.0f, 1.0f, 1.0f);
		tint.SetValuePtr3(1, &tnt[0]);

		if (collide)
			collide->Draw();
		
		tnt[0] = 0.0f;
		tnt[1] = 0.0f;
		tnt[2] = 0.7f;
		tint.SetValuePtr3(1, &tnt[0]);

		if (invis)
			invis->Draw();

		glfwSwapBuffers(win);
		glfwPollEvents();

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
