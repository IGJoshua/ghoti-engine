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

#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>

static lua_State *L;
static SDL_GameController *controller;

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
		// stack: keyboard keytable/nil
		if (lua_isnil(L, -1))
		{
			lua_pop(L, 1);

			lua_pushnumber(L, key);
			lua_createtable(L, 0, 2);

			lua_settable(L, -3);

			lua_pushnumber(L, key);
			lua_gettable(L, -2);
		}
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
		lua_checkstack(L, 3);

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

void handleSDLEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		if (!L)
		{
			continue;
		}

		if (event.cdevice.which == 0)
		{
			lua_getglobal(L, "engine");
			lua_getfield(L, -1, "gamepad");
			lua_remove(L, -2);

			switch (event.type)
			{
			case SDL_CONTROLLERAXISMOTION:
			{
				if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT
					|| event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
				{
					if (event.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
					{
						lua_getfield(L, -1, "lefttrigger");
					}
					else
					{
						lua_getfield(L, -1, "righttrigger");
					}

					// stack: gamepad trigger

					lua_getfield(L, -1, "deadzone");
					real64 deadzone = lua_tonumber(L, -1);
					lua_pop(L, 1);

					if (event.caxis.value < -SDL_MAX_SINT16 * deadzone
						|| event.caxis.value > SDL_MAX_SINT16 * deadzone)
					{
						lua_pushnumber(
							L,
							(real64)event.caxis.value / (real64)SDL_MAX_SINT16);
						lua_setfield(L, -2, "value");
					}
					else
					{
						lua_pushnumber(L, 0);
						lua_setfield(L, -2, "value");
					}

					lua_pop(L, 1);
				}
				else
				{
					// TODO: Implement a proper dead zone
					switch (event.caxis.axis)
					{
					case SDL_CONTROLLER_AXIS_LEFTX:
					case SDL_CONTROLLER_AXIS_LEFTY:
					{
						lua_getfield(L, -1, "leftstick");
					} break;
					case SDL_CONTROLLER_AXIS_RIGHTX:
					case SDL_CONTROLLER_AXIS_RIGHTY:
					{
						lua_getfield(L, -1, "rightstick");
					} break;
					}

					// stack: gamepad stick

					// Check deadzone type
					lua_getfield(L, -1, "deadzone");

					lua_getfield(L, -1, "type");
					bool circularDeadzone = false;

					{
						const char *str = lua_tostring(L, -1);
						if (!strcmp(str, "circular"))
						{
							circularDeadzone = true;
						}
					}

					lua_pop(L, 1);

					lua_getfield(L, -1, "value");
					real64 deadzone = lua_tonumber(L, -1);
					lua_pop(L, 2);

					// stack: gamepad stick

					lua_pushnumber(
						L,
						(real64)event.caxis.value
						/ (real64)SDL_MAX_SINT16);

					switch (event.caxis.axis)
					{
					case SDL_CONTROLLER_AXIS_LEFTX:
					case SDL_CONTROLLER_AXIS_RIGHTX:
					{
						lua_setfield(L, -2, "rawx");
					} break;
					case SDL_CONTROLLER_AXIS_LEFTY:
					case SDL_CONTROLLER_AXIS_RIGHTY:
					{
						lua_setfield(L, -2, "rawy");
					} break;
					}

					if (circularDeadzone)
					{
						// Get the angle of the thumbstick
						kmVec2 direction;

						switch (event.caxis.axis)
						{
						case SDL_CONTROLLER_AXIS_LEFTX:
						case SDL_CONTROLLER_AXIS_RIGHTX:
						{
							lua_getfield(L, -1, "rawy");
							direction.y = lua_tonumber(L, -1);
							lua_pop(L, 1);

							direction.x = (real32)event.caxis.value
								/ (real32)SDL_MAX_SINT16;
						} break;
						case SDL_CONTROLLER_AXIS_LEFTY:
						case SDL_CONTROLLER_AXIS_RIGHTY:
						{
							lua_getfield(L, -1, "rawx");
							direction.x = lua_tonumber(L, -1);
							lua_pop(L, 1);

							direction.y = (real32)event.caxis.value
								/ (real32)SDL_MAX_SINT16;
						} break;
						}

						if (kmVec2Length(&direction) > deadzone)
						{
							lua_pushnumber(L, direction.x);
							lua_setfield(L, -2, "x");
							lua_pushnumber(L, direction.y);
							lua_setfield(L, -2, "y");
						}
						else
						{
							lua_pushnumber(L, 0);
							lua_setfield(L, -2, "x");
							lua_pushnumber(L, 0);
							lua_setfield(L, -2, "y");
						}
					}
					else
					{
						if (event.caxis.value < -SDL_MAX_SINT16 * deadzone
							|| event.caxis.value > SDL_MAX_SINT16 * deadzone)
						{
							lua_pushnumber(
								L,
								(real64)event.caxis.value
								/ (real64)SDL_MAX_SINT16);
						}
						else
						{
							lua_pushnumber(
								L,
								0);
						}
						switch (event.caxis.axis)
						{
						case SDL_CONTROLLER_AXIS_LEFTX:
						case SDL_CONTROLLER_AXIS_RIGHTX:
						{
							lua_setfield(L, -2, "x");
						} break;
						case SDL_CONTROLLER_AXIS_LEFTY:
						case SDL_CONTROLLER_AXIS_RIGHTY:
						{
							lua_setfield(L, -2, "y");
						} break;
						}
					}

					lua_pop(L, 1);
				}
			} break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				switch (event.cbutton.button)
				{
				case SDL_CONTROLLER_BUTTON_A:
				case SDL_CONTROLLER_BUTTON_B:
				case SDL_CONTROLLER_BUTTON_X:
				case SDL_CONTROLLER_BUTTON_Y:
				case SDL_CONTROLLER_BUTTON_START:
				case SDL_CONTROLLER_BUTTON_BACK:
				case SDL_CONTROLLER_BUTTON_GUIDE:
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				case SDL_CONTROLLER_BUTTON_LEFTSTICK:
				case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				{
					lua_getfield(L, -1, "buttons");
				} break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				{
					lua_getfield(L, -1, "dpad");
				} break;
				}

				switch (event.cbutton.button)
				{
				case SDL_CONTROLLER_BUTTON_A:
				{
					lua_getfield(L, -1, "a");
				} break;
				case SDL_CONTROLLER_BUTTON_B:
				{
					lua_getfield(L, -1, "b");
				} break;
				case SDL_CONTROLLER_BUTTON_X:
				{
					lua_getfield(L, -1, "x");
				} break;
				case SDL_CONTROLLER_BUTTON_Y:
				{
					lua_getfield(L, -1, "y");
				} break;
				case SDL_CONTROLLER_BUTTON_START:
				{
					lua_getfield(L, -1, "start");
				} break;
				case SDL_CONTROLLER_BUTTON_BACK:
				{
					lua_getfield(L, -1, "back");
				} break;
				case SDL_CONTROLLER_BUTTON_GUIDE:
				{
					lua_getfield(L, -1, "guide");
				} break;
				case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
				{
					lua_getfield(L, -1, "leftbumper");
				} break;
				case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
				{
					lua_getfield(L, -1, "rightbumper");
				} break;
				case SDL_CONTROLLER_BUTTON_LEFTSTICK:
				{
					lua_getfield(L, -1, "leftstick");
				} break;
				case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
				{
					lua_getfield(L, -1, "rightstick");
				} break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
				{
					lua_getfield(L, -1, "down");
				} break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
				{
					lua_getfield(L, -1, "right");
				} break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
				{
					lua_getfield(L, -1, "up");
				} break;
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
				{
					lua_getfield(L, -1, "left");
				} break;
				}

				lua_remove(L, -2);

				lua_pushboolean(L, event.cbutton.state);
				lua_setfield(L, -2, "keydown");

				lua_pushboolean(L, 1);
				lua_setfield(L, -2, "updated");

				lua_pop(L, 1);
			} break;
			case SDL_CONTROLLERDEVICEADDED:
			{
				if (event.cdevice.which == 0)
				{
					controller = SDL_GameControllerOpen(0);
				}
			} break;
			case SDL_CONTROLLERDEVICEREMOVED:
			{
				if (event.cdevice.which == 0 && controller)
				{
					SDL_GameControllerClose(controller);
				}
			} break;
			default:
			{
			} break;
			}

			lua_pop(L, 1);
		}
	}
}

int32 main()
{
	if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0)
	{
		printf("Failed to initialize SDL2.\n%s\n", SDL_GetError());
		return -1;
	}

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

	SDL_JoystickEventState(SDL_ENABLE);
	SDL_GameControllerEventState(SDL_ENABLE);
	controller = SDL_GameControllerOpen(0);

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
			handleSDLEvents();
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

	SDL_Quit();

	return 0;
}
