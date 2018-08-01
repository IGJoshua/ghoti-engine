#include "defines.h"

#include "core/log.h"
#include "core/window.h"
#include "core/input.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

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

#include <ode/ode.h>

#include <time.h>
#include <stdlib.h>

extern lua_State *L;
extern real64 alpha;
extern List activeScenes;
extern bool changeScene;
extern bool reloadingScene;
extern List unloadedScenes;
extern bool loadingSave;
extern List savedScenes;

int32 main()
{
	srand(time(0));

	initLog();

	GLFWwindow *window = initWindow(640, 480, "Ghoti");

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
	savedScenes = createList(sizeof(char*));

	dInitODE();

	glfwMakeContextCurrent(window);
	glfwSwapInterval(VSYNC);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	initSystems();

	deleteFolder(RUNTIME_STATE_DIR, false);

	// Init Lua
	L = luaL_newstate();
	luaL_openlibs(L);

	int luaError = 0;

#ifndef _DEBUG
	if (L)
	{
		lua_getglobal(L, "io");
		lua_getfield(L, -1, "output");
		lua_remove(L, -2);
		lua_pushstring(L, "lua.log");
		luaError = lua_pcall(L, 1, 0, 0);
		if (luaError)
		{
			LOG("Lua error: %s\n", lua_tostring(L, -1));
			lua_close(L);
			L = 0;
		}
	}
#endif

	luaError = luaL_loadfile(L, "resources/scripts/engine.lua")
		|| lua_pcall(L, 0, 0, 0);
	if (luaError)
	{
		LOG("Lua Error: %s\n", lua_tostring(L, -1));
		lua_pop(L, 1);

		lua_close(L);
		freeWindow(window);
		return 1;
	}

	// State previous
	// State next

	ListIterator itr = {};

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timestep
	real64 dt = 1.0 / 24.0;

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

		while (accumulator >= dt && !glfwWindowShouldClose(window))
		{
			for (itr = listGetIterator(&activeScenes);
				 !listIteratorAtEnd(itr);
				 listMoveIterator(&itr))
			{
				Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

				if (scene->loadedThisFrame)
				{
					scene->loadedThisFrame = false;
					continue;
				}

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
						LOG("Lua error: %s\n", lua_tostring(L, -1));
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
					LOG("Lua error: %s\n", lua_tostring(L, -1));
					lua_close(L);
					L = 0;
				}
			}

			if (loadingSave)
			{
				bool isReloadingScene = reloadingScene;

				for (ListIterator i = listGetIterator(&activeScenes);
					 !listIteratorAtEnd(i);
					 listMoveIterator(&i))
				{
					Scene **scene = LIST_ITERATOR_GET_ELEMENT(Scene*, i);

					shutdownScene(scene);

					reloadingScene = true;
					freeScene(scene);
					reloadingScene = false;
				}

				listClear(&activeScenes);

				for (ListIterator i = listGetIterator(&savedScenes);
					 !listIteratorAtEnd(i);
					 listMoveIterator(&i))
				{
					char *name = *LIST_ITERATOR_GET_ELEMENT(char*, i);
					loadScene(name);
					free(name);
				}

				listClear(&savedScenes);

				reloadingScene = isReloadingScene;
				loadingSave = false;
			}
			else if (changeScene)
			{
				for (ListIterator i = listGetIterator(&unloadedScenes);
					 !listIteratorAtEnd(i);)
				{
					Scene **scene = LIST_ITERATOR_GET_ELEMENT(Scene*, i);

					char *name = NULL;
					if (reloadingScene)
					{
						name = malloc(strlen((*scene)->name) + 1);
						strcpy(name, (*scene)->name);
					}

					if (deactivateScene(*scene) == -1)
					{
						continue;
					}

					shutdownScene(scene);
					freeScene(scene);
					listRemove(&unloadedScenes, &i);

					if (reloadingScene)
					{
						loadScene(name);
						free(name);
					}
				}

				changeScene = false;
				reloadingScene = false;
			}

			// Integrate current state over t to dt (so, update)
			t += dt;
			accumulator -= dt;

			inputHandleEvents();
		}

		alpha = accumulator / dt;

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

			CameraComponent *camera = sceneGetComponentFromEntity(
				scene,
				scene->mainCamera,
				idFromName("camera"));
			if (camera)
			{
				camera->aspectRatio = aspectRatio;
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
					LOG("Lua error: %s\n", lua_tostring(L, -1));
					lua_close(L);
					L = 0;
				}
			}

			sceneRunRenderFrameSystems(scene, frameTime);
		}

		glfwSwapBuffers(window);
	}

	reloadingScene = true;

	for (itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene *, itr);

		shutdownScene(&scene);
		freeScene(&scene);
	}

	deleteFolder(RUNTIME_STATE_DIR, false);

	if (L)
	{
		lua_close(L);
	}

	freeSystems();

	dCloseODE();

	shutdownInput();

	freeWindow(window);

	shutdownLog();

	return 0;
}
