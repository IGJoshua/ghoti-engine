#include "core/window.h"
#include "core/config.h"
#include "core/log.h"

#include "asset_management/texture.h"

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

internal GLFWwindow *wnd;
internal GLFWimage icon;

internal bool isVSYNCEnabled;

internal int32 x, y, w, h;
internal bool isFullscreen;

extern Config config;

extern int32 viewportWidth;
extern int32 viewportHeight;

internal GLFWmonitor* getActiveMonitor(void);

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

	glfwWindowHint(GLFW_SAMPLES, config.graphicsConfig.numMSAASamples);

	glfwWindowHint(GLFW_MAXIMIZED, config.windowConfig.maximized);
	glfwWindowHint(GLFW_RESIZABLE, config.windowConfig.resizable);

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

	TextureData iconData;
	if (loadTextureData(
		ASSET_LOG_TYPE_NONE,
		"icon",
		NULL,
		config.windowConfig.icon,
		4,
		&iconData) != -1)
	{
		icon.width = iconData.width;
		icon.height = iconData.height;
		icon.pixels = iconData.data;
		glfwSetWindowIcon(window, 1, &icon);
	}
	else
	{
		icon.pixels = NULL;
	}

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
		GLFWmonitor *monitor = getActiveMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);

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

	switchVSYNCMode();
	switchVSYNCMode();

	isFullscreen = fullscreen;
}

void getViewportSize(int32 *width, int32 *height)
{
	*width = viewportWidth;
	*height = viewportHeight;
}

void setWindowPosition(int32 xPosition, int32 yPosition)
{
	glfwSetWindowPos(wnd, xPosition, yPosition);
	glfwGetWindowPos(wnd, &x, &y);
}

void setWindowSize(int32 width, int32 height)
{
	glfwSetWindowSize(wnd, width, height);
	glfwGetWindowSize(wnd, &w, &h);
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

	free(icon.pixels);

	return 0;
}

GLFWmonitor* getActiveMonitor(void)
{
	GLFWmonitor *activeMonitor = NULL;

	glfwGetWindowPos(wnd, &x, &y);
	glfwGetWindowSize(wnd, &w, &h);

	int32 numMonitors;
	GLFWmonitor **monitors = glfwGetMonitors(&numMonitors);

	int32 maxOverlap = 0;
	for (uint32 i = 0; i < numMonitors; i++)
	{
		GLFWmonitor *monitor = monitors[i];

		int32 mX, mY;
		glfwGetMonitorPos(monitor, &mX, &mY);

		const GLFWvidmode *mode = glfwGetVideoMode(monitor);

		int32 overlap =
			MAX(0, MIN(x + w, mX + mode->width) - MAX(x, mX)) *
			MAX(0, MIN(y + h, mY + mode->height) - MAX(y, mY));

		if (overlap > maxOverlap)
		{
			maxOverlap = overlap;
			activeMonitor = monitor;
		}
	}

	if (!activeMonitor)
	{
		activeMonitor = glfwGetPrimaryMonitor();
	}

	return activeMonitor;
}