#include "defines.h"
#include "core/window.h"

#include "renderer/renderer_types.h"
#include "renderer/mesh.h"

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>

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
	glfwSwapInterval(VSYNC);
	glfwSetKeyCallback(window, &keyCallback);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timesetep
	real64 dt = 0.01;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

	// State previous
	// State next

	// TODO: Remove stupid stuff that's just for testing
	Mesh *m;
	loadMesh(&m, "resource/mesh/teapot.dae", 0);

	while(!glfwWindowShouldClose(window))
	{
		// Start timestep
		real64 newTime = glfwGetTime();
		real64 frameTime = newTime - currentTime;
		if (frameTime > 0.25)
		{
			frameTime = 0.25;
		}
		currentTime = newTime;

		accumulator += frameTime;

		while (accumulator >= dt)
		{
			// TODO: State chates
			// TODO: App update
			// Previous state = currentState
			// Integrate current state over t to dt (so, update)
			t += dt;
			accumulator -= dt;
		}

		const real64 alpha = accumulator / dt;

		// Lerp state between previous and next

		char title[128];
		sprintf(title, "Monochrome FPS: %f MS/F: %f", 1 / frameTime, frameTime * 1000);
		if (fmodf((real32)currentTime, 1.0f) < 0.001)
		{
			glfwSetWindowTitle(window, title);
		}

		// Render
		// TODO: Real render code
		int32 width, height;
		real32 aspect;

		glfwGetFramebufferSize(window, &width, &height);

		aspect = (real32)width / (real32)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	freeMesh(&m);
	freeWindow(window);

    return 0;
}
