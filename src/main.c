#include "defines.h"
#include "window.h"

#include <GLFW/glfw3.h>

#include <stdio.h>

void keyCallback(
	GLFWwindow *window,
	int key,
	int scancode,
	int action,
	int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

int main()
{
	GLFWwindow *window = initWindow(640, 480, "Monochrome");

	if (!window)
	{
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, &keyCallback);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while(!glfwWindowShouldClose(window))
	{
		int32 width, height;
		float32 aspect;

		glfwGetFramebufferSize(window, &width, &height);

		aspect = (float32)width / (float32)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	freeWindow(window);

    return 0;
}
