#include "core/window.h"
#include "core/config.h"
#include "core/log.h"

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

internal GLFWwindow *wnd;

internal bool isVSYNCEnabled;

internal int32 x, y, w, h;
internal bool isFullscreen;

extern Config config;

extern int32 viewportWidth;
extern int32 viewportHeight;

internal
void errorCallback(
	int error,
	const char *description)
{
	LOG("GLFW Error %d: %s\n", error, description);
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

	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);

	wnd = window;
	viewportWidth = width;
	viewportHeight = height;

	isFullscreen = false;

	setVSYNCMode(config.windowConfig.vsync);
	setFullscreenMode(config.windowConfig.fullscreen);

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();

	if (GLEW_OK != glewError)
	{
		LOG("Error: %s\n", glewGetErrorString(glewError));
	}

	return window;
}

void setMousePosition(real64 x, real64 y)
{
	glfwSetCursorPos(wnd, x, y);
}

void setMouseHidden(bool hidden)
{
	glfwSetInputMode(
		wnd,
		GLFW_CURSOR,
		hidden ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

void setMouseLocked(bool locked)
{
	glfwSetInputMode(
		wnd,
		GLFW_CURSOR,
		locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool getVSYNCMode(void)
{
	return isVSYNCEnabled;
}

void switchVSYNCMode(void)
{
	setVSYNCMode(!isVSYNCEnabled);
}

void setVSYNCMode(bool vsync)
{
	if (vsync)
	{
		glfwSwapInterval(1);
	}
	else
	{
		glfwSwapInterval(0);
	}

	isVSYNCEnabled = vsync;
}

bool getFullscreenMode(void)
{
	return isFullscreen;
}

void switchFullscreenMode(void)
{
	setFullscreenMode(!isFullscreen);
}

void setFullscreenMode(bool fullscreen)
{
	if (fullscreen && !isFullscreen)
	{
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);

		glfwGetWindowPos(wnd, &x, &y);
		glfwGetWindowSize(wnd, &w, &h);

		glfwSetWindowMonitor(
			wnd,
			monitor,
			0,
			0,
			mode->width,
			mode->height,
			GLFW_DONT_CARE);
	}
	else if (isFullscreen)
	{
		glfwSetWindowMonitor(wnd, NULL, x, y, w, h, GLFW_DONT_CARE);
	}

	isFullscreen = fullscreen;
}

void getViewportSize(int32 *width, int32 *height)
{
	*width = viewportWidth;
	*height = viewportHeight;
}

int32 closeWindow(void)
{
	glfwSetWindowShouldClose(wnd, 1);
	return 0;
}

int32 freeWindow(
	GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();

	wnd = 0;

	return 0;
}
