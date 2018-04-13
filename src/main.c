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
	glfwSwapInterval(VSYNC);
	glfwSetKeyCallback(window, &keyCallback);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// total accumulated fixed timestep
	double t = 0.0;
	// Fixed timesetep
	double dt = 0.01;

	double currentTime = glfwGetTime();
	double accumulator = 0.0;

	// State previous
	// State next

	while(!glfwWindowShouldClose(window))
	{
		// Start timestep
		double newTime = glfwGetTime();
		double frameTime = newTime - currentTime;
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

		const double alpha = accumulator / dt;

		// Lerp state between previous and next

		// Render
		// TODO: Real render code
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
