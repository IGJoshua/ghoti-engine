#include "defines.h"
#include "core/window.h"

#include "asset_management/model.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

#include "components/component_types.h"

#include "systems.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <kazmath/mat4.h>

#include <luajit-2.0/lua.h>
#include <luajit-2.0/lauxlib.h>
#include <luajit-2.0/lualib.h>

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

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

typedef struct orbit_component_t
{
	kmVec3 origin;
	float speed;
	float radius;
} OrbitComponent;

int32 main()
{
	srand(time(0));

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

	// Init Lua
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int luaError = luaL_loadfile(L, "resources/scripts/engine.lua") || lua_pcall(L, 0, 0, 0);
	if (luaError)
	{
		printf("Lua Error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_close(L);
		freeWindow(window);
		return 1;
	}

	// TODO: Setup basic component state
	Scene *scene;
	if (loadScene("scene_1", &scene) == -1)
	{
		return -1;
	}

	// Add component types

	// Add systems
	System rendererSystem = createRendererSystem();
	sceneAddRenderFrameSystem(scene, rendererSystem);

	// Create entities

	// State previous
	// State next

	sceneInitSystems(scene);
	sceneInitLua(&L, scene);

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timestep
	real64 dt = 1.0 / 60.0;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

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
			// TODO: Previous state = currentState
			sceneRunPhysicsFrameSystems(scene, dt);

			// Load the lua engine table and run its physics systems
			if (L)
			{
				lua_getglobal(L, "engine");
				lua_getfield(L, -1, "runPhysicsSystems");
				lua_remove(L, -2);
				lua_pushlightuserdata(L, scene);
				lua_pushnumber(L, dt);
				luaError = lua_pcall(L, 2, 0, 0);
				if (luaError)
				{
					printf("Lua error: %s\n", lua_tostring(L, -1));
					lua_close(L);
					L = 0;
				}
			}

			// Integrate current state over t to dt (so, update)
			t += dt;
			accumulator -= dt;
		}

		// const real64 alpha = accumulator / dt;

		// Lerp state between previous and next

		int32 width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		real32 aspectRatio = (real32)width / (real32)height;

		CameraComponent *cam = sceneGetComponentFromEntity(scene, scene->mainCamera, idFromName("camera"));
		if (cam)
		{
			cam->aspectRatio = aspectRatio;
		}

		// Render
		if (L)
		{
			lua_getglobal(L, "engine");
			lua_getfield(L, -1, "runRenderSystems");
			lua_remove(L, -2);
			lua_pushlightuserdata(L, scene);
			lua_pushnumber(L, frameTime);
			luaError = lua_pcall(L, 2, 0, 0);
			if (luaError)
			{
				printf("Lua error: %s\n", lua_tostring(L, -1));
				lua_close(L);
				L = 0;
			}
		}

		sceneRunRenderFrameSystems(scene, frameTime);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	if (L)
	{
		sceneShutdownLua(&L, scene);
	}
	sceneShutdownSystems(scene);
	freeScene(&scene);

	if (L)
	{
		lua_close(L);
	}

	freeWindow(window);

	return 0;
}
