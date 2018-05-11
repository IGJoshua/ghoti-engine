#include "defines.h"
#include "core/window.h"

#include "asset_management/asset_manager.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <kazmath/mat4.h>

#include <stdio.h>
#include <math.h>
#include <malloc.h>

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

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timesetep
	real64 dt = 1.0 / 30.0;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

	// State previous
	// State next
	Scene *scene; 
  	loadScene("scene_1", &scene); 

	initRenderer();

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

		int32 width, height;
		real32 aspect;

		glfwGetFramebufferSize(window, &width, &height);

		aspect = (real32)width / (real32)height;

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Render
		for (uint32 i = 0; i < scene->numModels; i++)
		{
			renderModel(scene->models[i]);
		}

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	unloadScene(scene);
	freeWindow(window);

	return 0;
}
