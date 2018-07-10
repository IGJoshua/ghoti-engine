#include "core/window.h"

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include <stdio.h>

static GLFWwindow *wnd;

internal
void errorCallback(
	int error,
	const char *description)
{
	printf("GLFW Error %d: %s\n", error, description);
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
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window)
	{
		glfwTerminate();

		return 0;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		printf("Error: %s\n", glewGetErrorString(err));
	}

	ilInit();
	iluInit();

	wnd = window;

	return window;
}

int32 closeWindow(void)
{
	glfwSetWindowShouldClose(wnd, 1);
	return 0;
}

int32 freeWindow(
	GLFWwindow *window)
{
	ilShutDown();

	glfwDestroyWindow(window);
	glfwTerminate();

	wnd = 0;

	return 0;
}
