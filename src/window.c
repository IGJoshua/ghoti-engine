#include "window.h"

#include <GLFW/glfw3.h>
#include <stdio.h>

internal
void errorCallback(
	int error,
	const char *description)
{
	fprintf(stderr, "Error code %d: %s\n", error, description);
}

GLFWwindow *initWindow(
	uint32 width,
	uint32 height,
	const char *title)
{
	if (!glfwInit())
	{
		return 0;
	}

	glfwSetErrorCallback(&errorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GLFW_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GLFW_VERSION_MINOR);
	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window)
	{
		glfwTerminate();

		return 0;
	}

	return window;
}

int32 freeWindow(
	GLFWwindow *window)
{
	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}
