#include "defines.h"
#include "core/window.h"

#include "asset_management/scene.h"
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

static lua_State *L;

void keyCallback(
	GLFWwindow *window,
	int key,
	int scancode,
	int action,
	int mods)
{
	if (L)
	{
		lua_checkstack(L, 4);

		// Get the keyboard
		lua_getglobal(L, "engine");
		lua_getfield(L, -1, "keyboard");
		lua_remove(L, -2);

		// Stack: keyboard

		// Check if the field is nil
		lua_pushnumber(L, key);
		lua_gettable(L, -2);
		// stack: keyboard keytable

		// Set the correct values in the table
		// The key is down (or not)
		lua_pushboolean(L, action != GLFW_RELEASE ? 1 : 0);
		// stack: keyboard keytable bool
		lua_setfield(L, -2, "keydown");
		// stack: keyboard keytable(with keydown set)

		// The key has just been updated
		lua_pushboolean(L, action == GLFW_PRESS || action == GLFW_RELEASE);
		// stack: keyboard keytable bool
		lua_setfield(L, -2, "updated");
		// stack: keyboard keytable(with updated set)

		lua_pop(L, 2);
		// stack: cleaned
	}
	else
	{
		printf("No lua state exists, failing to register keypress\n");
	}
}

void cursorPositionCallback(
	GLFWwindow *window,
	double xpos,
	double ypos)
{
	if (L)
	{
		lua_checkstack(L, 3);

		lua_getglobal(L, "engine");
		lua_getfield(L, -1, "mouse");
		lua_remove(L, -2);
		// stack: mouse

		lua_pushnumber(L, xpos);
		lua_setfield(L, -2, "x");

		lua_pushnumber(L, ypos);
		lua_setfield(L, -2, "y");
	}
	else
	{
		printf("No lua state exists, failing to register cursor position\n");
	}
}

void mouseButtonCallback(
	GLFWwindow *window,
	int button,
	int action,
	int mods)
{
	if (L)
	{
		lua_checkstack(L, 4);

		lua_getglobal(L, "engine");
		lua_getfield(L, -1, "mouse");
		lua_remove(L, -2);
		lua_getfield(L, -1, "buttons");
		lua_remove(L, -2);
		// stack: buttons

		++button;

		// Get the proper table
		lua_pushnumber(L, button);
		// stack: buttons button_number
		lua_gettable(L, -2);
		// stack: buttons button_table/nil
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);

			lua_pushnumber(L, button);
			lua_createtable(L, 0, 2);

			lua_pushboolean(L, 0);
			lua_setfield(L, -2, "keydown");

			lua_pushboolean(L, 0);
			lua_setfield(L, -2, "updated");

			lua_settable(L, -3);

			lua_pushnumber(L, button);
			lua_gettable(L, -2);
		}
		// stack: buttons button_table

		lua_pushboolean(L, action == GLFW_PRESS);
		lua_setfield(L, -2, "keydown");

		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "updated");
	}
	else
	{
		printf("No lua state exists, failing to register mouse click\n");
	}
}

void mouseScrollCallback(
	GLFWwindow *window,
	double xoffset,
	double yoffset)
{
	if (L)
	{
		lua_checkstack(L, 3); // TODO: Get a valid size here

		lua_getglobal(L, "engine");
		lua_getfield(L, -1, "mouse");
		lua_remove(L, -2);
		lua_getfield(L, -1, "scroll");
		lua_remove(L, -2);
		// stack: scroll

		lua_pushnumber(L, xoffset);
		lua_setfield(L, -2, "x");

		lua_pushnumber(L, yoffset);
		lua_setfield(L, -2, "y");
	}
	else
	{
		printf("No lua state exists, failing to register mouse scroll\n");
	}
}

int32 main()
{
	GLFWwindow *window = initWindow(640, 480, "Monochrome");

	if (!window)
	{
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(VSYNC);
	glfwSetKeyCallback(window, &keyCallback);
	glfwSetCursorPosCallback(window, &cursorPositionCallback);
	glfwSetMouseButtonCallback(window, &mouseButtonCallback);
	glfwSetScrollCallback(window, &mouseScrollCallback);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// Init Lua
	L = luaL_newstate();
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
	Scene *scene = createScene();

	// Add component types

	// Add systems
	System rendererSystem = createRendererSystem();

	sceneAddRenderFrameSystem(scene, rendererSystem);

	// Create entities

	// State previous
	// State next

	// TODO: Make this thing work
  	//loadScene("scene_1", &scene);

	sceneInitSystems(scene);
	sceneInitLua(&L, scene);

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timesetep
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

			// Clean input
			if (L)
			{
				lua_getglobal(L, "engine");
				lua_getfield(L, -1, "cleanInput");
				lua_remove(L, -2);
				luaError = lua_pcall(L, 0, 0, 0);
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

			glfwPollEvents();
		}

		const real64 alpha = accumulator / dt;

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
