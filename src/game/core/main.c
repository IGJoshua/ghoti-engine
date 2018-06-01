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

typedef struct name_component_t
{
	char name[64];
} NameComponent;

typedef struct orbit_component_t
{
	kmVec3 origin;
	float speed;
	float radius;
} OrbitComponent;

void moveSystem(Scene *scene, UUID entityID)
{
	UUID transID = idFromName("transform");
	TransformComponent *transform =
		sceneGetComponentFromEntity(scene, entityID, transID);

	UUID orbitID = idFromName("orbit");
	OrbitComponent *orbit =
		sceneGetComponentFromEntity(scene, entityID, orbitID);

	transform->position.x =
		sinf(glfwGetTime() * orbit->speed)
		* orbit->radius
		+ orbit->origin.x;
	transform->position.y =
		cosf(glfwGetTime() * orbit->speed)
		* orbit->radius
		+ orbit->origin.y;
	transform->position.z = orbit->origin.z;
}

void nameSystem(Scene *scene, UUID entityID)
{
	UUID nameID = idFromName("name");
	NameComponent *name =
		sceneGetComponentFromEntity(scene, entityID, nameID);
}

void cameraOrbit(Scene *scene, UUID entityID)
{
	UUID transID = idFromName("transform");
	TransformComponent *transform =
		sceneGetComponentFromEntity(scene, entityID, transID);

	transform->position.x = sinf(glfwGetTime() + 2.0f);
}

int32 int32Comp(void *a, void *b)
{
	return *(uint32 *)a != *(uint32 *)b;
}

#include <stdlib.h>
#include <time.h>

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

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

	// total accumulated fixed timestep
	real64 t = 0.0;
	// Fixed timesetep
	real64 dt = 1.0 / 60.0;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

	// Init Lua
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	int luaError = luaL_loadfile(L, "resources/scripts/main.lua") || lua_pcall(L, 0, 0, 0);
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
	UUID transformComponentID = idFromName("transform");
	sceneAddComponentType(scene, transformComponentID, sizeof(TransformComponent), 4);

	UUID orbitComponentID = idFromName("orbit");
	sceneAddComponentType(scene, orbitComponentID, sizeof(OrbitComponent), 4);

	UUID nameComponentID = idFromName("name");
	sceneAddComponentType(scene, nameComponentID, sizeof(NameComponent), 4);

	UUID modelComponentID = idFromName("model");
	sceneAddComponentType(scene, modelComponentID, sizeof(ModelComponent), 4);

	UUID cameraComponentID = idFromName("camera");
	sceneAddComponentType(scene, cameraComponentID, sizeof(CameraComponent), 2);

	// Add systems
	// TODO: Invent way to register systems
	List movementComponents = createList(sizeof(UUID));
	listPushBack(&movementComponents, &orbitComponentID);
	listPushBack(&movementComponents, &transformComponentID);
	System movementSystem = createSystem(movementComponents, 0, &moveSystem, 0);

	sceneAddPhysicsFrameSystem(scene, movementSystem);

	List nameComponents = createList(sizeof(UUID));
	listPushBack(&nameComponents, &nameComponentID);
	System printNameSystem = createSystem(nameComponents, 0, &nameSystem, 0);

	System rendererSystem = createRendererSystem();

	sceneAddRenderFrameSystem(scene, rendererSystem);

	List cameraComponents = createList(sizeof(UUID));
	listPushBack(&cameraComponents, &cameraComponentID);
	listPushBack(&cameraComponents, &transformComponentID);
	System cameraSystem = createSystem(cameraComponents, 0, &cameraOrbit, 0);

	sceneAddRenderFrameSystem(scene, cameraSystem);

	// Create entities
	UUID entity1 = idFromName("ENTITY1");
	sceneRegisterEntity(scene, entity1);
	UUID entity2 = idFromName("ENTITY2");
	sceneRegisterEntity(scene, entity2);

	UUID teapot = idFromName("TEAPOT");
	sceneRegisterEntity(scene, teapot);

	UUID test = idFromName("test");
	sceneRegisterEntity(scene, test);

	UUID camera = idFromName("CAMERA");
	sceneRegisterEntity(scene, camera);

	scene->mainCamera = camera;

	TransformComponent transform = {};
	transform.position.x = 1.0f;
	transform.position.y = 1.0f;
	transform.position.z = 1.0f;
	sceneAddComponentToEntity(scene, entity1, transformComponentID, &transform);

	NameComponent name = {};
	strcpy(name.name, "Hello, world!");
	sceneAddComponentToEntity(scene, entity2, nameComponentID, &name);

	ModelComponent teapotModel = {};
	strcpy(teapotModel.name, "teapot");
	sceneAddComponentToEntity(scene, teapot, modelComponentID, &teapotModel);
	OrbitComponent orbitPosition = {};
	kmVec3Zero(&orbitPosition.origin);
	orbitPosition.radius = 2.0f;
	orbitPosition.speed = 3.0f;
	sceneAddComponentToEntity(scene, teapot, orbitComponentID, &orbitPosition);
	kmVec3Zero(&transform.position);
	transform.scale.x = 0.01f;
	transform.scale.y = 0.01f;
	transform.scale.z = 0.01f;
	kmQuaternionRotationPitchYawRoll(&transform.rotation, kmDegreesToRadians(90), 0, 0);
	sceneAddComponentToEntity(scene, teapot, transformComponentID, &transform);

	ModelComponent testModel = {};
	strcpy(testModel.name, "test");
	sceneAddComponentToEntity(scene, test, modelComponentID, &testModel);
	kmVec3Zero(&transform.position);
	transform.scale.x = 1;
	transform.scale.y = 1;
	transform.scale.z = 1;
	kmQuaternionRotationPitchYawRoll(&transform.rotation, kmDegreesToRadians(90), 0, 0);
	sceneAddComponentToEntity(scene, test, transformComponentID, &transform);

	CameraComponent cameraComp = {};
	cameraComp.aspectRatio = 4.0f / 3.0f;
	cameraComp.fov = 80;
	cameraComp.nearPlane = 0.1f;
	cameraComp.farPlane = 1000.0f;
	cameraComp.projectionType = CAMERA_PROJECTION_TYPE_PERSPECTIVE;
	sceneAddComponentToEntity(scene, camera, cameraComponentID, &cameraComp);
	kmVec3Fill(&transform.position, 0, 0, 2);
	transform.scale.x = 1;
	transform.scale.y = 1;
	transform.scale.z = 1;
	kmQuaternionIdentity(&transform.rotation);
	sceneAddComponentToEntity(scene, camera, transformComponentID, &transform);

	// State previous
	// State next

	// TODO: Make this thing work
  	//loadScene("scene_1", &scene);

	sceneInitSystems(scene);

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
			// Previous state = currentState
			// TODO: State chates
			// TODO: App update
			sceneRunPhysicsFrameSystems(scene);

			// Integrate current state over t to dt (so, update)
			t += dt;
			accumulator -= dt;
		}

		const real64 alpha = accumulator / dt;

		// Lerp state between previous and next

		int32 width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		real32 aspectRatio = (real32)width / (real32)height;

		CameraComponent *cam = sceneGetComponentFromEntity(scene, scene->mainCamera, cameraComponentID);
		if (cam)
		{
			cam->aspectRatio = aspectRatio;
		}

		// Render
		sceneRunRenderFrameSystems(scene);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	freeModel("teapot");
	freeModel("test");

	freeSystem(&movementSystem);
	freeSystem(&printNameSystem);
	freeSystem(&cameraSystem);
	freeRendererSystem(&rendererSystem);

	freeScene(&scene);

	lua_close(L);

	freeWindow(window);

	return 0;
}
