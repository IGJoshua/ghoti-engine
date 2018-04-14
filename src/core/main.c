#include "defines.h"
#include "core/window.h"

#include "renderer/renderer_types.h"
#include "renderer/mesh.h"
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
	real64 dt = 0.01;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

	// State previous
	// State next

	// TODO: Remove stupid stuff that's just for testing
	Mesh *m;
	loadMesh(&m, "resources/meshes/teapot.dae", 0);

	Shader vertShader = compileShaderFromFile("resources/shaders/base.vert", SHADER_VERTEX);
	printf("Value of the vert shader location: %d\n", vertShader.object);
	Shader fragShader = compileShaderFromFile("resources/shaders/color.frag", SHADER_FRAGMENT);
	printf("Value of the frag shader location: %d\n", fragShader.object);

	ShaderPipeline pipeline;
	{
		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		pipeline = composeShaderPipeline(program, 2);
	}
	printf("Value of the shader pipeline location: %d\n", pipeline.object);

	freeShader(vertShader);
	freeShader(fragShader);
	free(pipeline.shaders);
	pipeline.shaderCount = 0;

	Uniform worldUniform = getUniform(pipeline, "model", UNIFORM_MAT4);
	printf("Value of the location for world uniform: %d\n", worldUniform.location);
	printf("Get model uniform: %s\n", gluErrorString(glGetError()));
	Uniform viewUniform = getUniform(pipeline, "view", UNIFORM_MAT4);
	printf("Value of the location for view uniform: %d\n", viewUniform.location);
	printf("Get view uniform: %s\n", gluErrorString(glGetError()));
	Uniform projectionUniform = getUniform(pipeline, "projection", UNIFORM_MAT4);
	printf("Value of the location for projection uniform: %d\n", projectionUniform.location);
	printf("Get projection uniform: %s\n", gluErrorString(glGetError()));

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
		// TODO: Real render code
		bindShaderPipeline(pipeline);

		kmMat4 projection;
		kmMat4PerspectiveProjection(&projection, 90, 4.0f / 3.0f, 0.1f, 1000.0f);
		kmMat4 world;
		kmMat4RotationX(&world, kmDegreesToRadians(-90));
		kmMat4 view;
		kmMat4Translation(&view, 0, 0, 150);
		kmMat4Inverse(&view, &view);

		setUniform(worldUniform, &world);
		setUniform(viewUniform, &view);
		setUniform(projectionUniform, &projection);

		renderMesh(m);

		glUseProgram(0);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	freeMesh(&m);
	freeWindow(window);

	return 0;
}
