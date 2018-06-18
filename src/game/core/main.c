#include "defines.h"

#include "core/window.h"
#include "core/input.h"

#include "asset_management/model.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"
#include "ECS/save.h"

#include "file/utilities.h"

#include "components/component_types.h"

#include "systems.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include <GL/glew.h>
#include <GL/glu.h>
#include <GLFW/glfw3.h>

#include <luajit-2.0/lua.h>
#include <luajit-2.0/lauxlib.h>
#include <luajit-2.0/lualib.h>

#include <SDL2/SDL.h>

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

extern lua_State *L;
extern List activeScenes;
extern uint32 changeScene;
extern List unloadedScenes;

int32 main()
{
	srand(time(0));

	GLFWwindow *window = initWindow(640, 480, "Monochrome");

	if (!window)
	{
		return -1;
	}

	int32 err = initInput(window);
	if (err)
	{
		freeWindow(window);
		return err;
	}

	activeScenes = createList(sizeof(Scene *));
	unloadedScenes = createList(sizeof(Scene *));

	glfwMakeContextCurrent(window);
	glfwSwapInterval(VSYNC);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	initSystems();

	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L);

	int luaError = luaL_loadfile(L, "resources/scripts/engine.lua")
		|| lua_pcall(L, 0, 0, 0);
	if (luaError)
	{
		printf("Lua Error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_close(L);
		freeWindow(window);
		return 1;
	}

	Scene *initScene;
	loadScene("scene_2", &initScene);
	listPushFront(&activeScenes, &initScene);

	// State previous
	// State next

	ListIterator itr = 0;
	for (itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

		sceneInitSystems(scene);
		sceneInitLua(&L, scene);
	}

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
			for (itr = listGetIterator(&activeScenes);
				 !listIteratorAtEnd(itr);
				 listMoveIterator(&itr))
			{
				Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

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

			if (changeScene)
			{
				// TODO: unload all the unneeded scenes
				for (ListIterator i = listGetIterator(&unloadedScenes);
					 !listIteratorAtEnd(i);
					 listMoveIterator(&i))
				{
					Scene **scene = ((Scene **)(&((*i)->data)));

					sceneShutdownLua(&L, *scene);
					sceneShutdownSystems(*scene);
					freeScene(scene);
				}
				listClear(&unloadedScenes);

				changeScene = 0;
			}

			// Integrate current state over t to dt (so, update)
			t += dt;
			accumulator -= dt;

			inputHandleEvents();
		}

		// const real64 alpha = accumulator / dt;

		// Lerp state between previous and next

		int32 width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		real32 aspectRatio = (real32)width / (real32)height;

		for (itr = listGetIterator(&activeScenes);
			 !listIteratorAtEnd(itr);
			 listMoveIterator(&itr))
		{
			Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

			CameraComponent *cam = sceneGetComponentFromEntity(
				scene,
				scene->mainCamera,
				idFromName("camera"));
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
		}

		glfwSwapBuffers(window);
	}

	for (itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

		if (L)
		{
			sceneShutdownLua(&L, scene);
		}
		sceneShutdownSystems(scene);
		freeScene(&scene);
	}

	if (deleteFolder(RUNTIME_STATE_DIR) == -1)
	{
		printf("Failed to delete runtime folder\n");
	}

	if (L)
	{
		lua_close(L);
	}

	freeSystems();

	shutdownInput();

	freeWindow(window);

	return 0;
}
