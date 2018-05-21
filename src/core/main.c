#include "defines.h"
#include "core/window.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"
#include "ECS/system.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <math.h>
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

typedef struct transform_component_t
{
	float position[3];
	float rotation[4];
} TransformComponent;

typedef struct name_component_t
{
	char name[64];
} NameComponent;

void moveSystem(Scene *scene, UUID entityID)
{
	UUID transID = {};
	strcpy(transID.string, "transform");
	TransformComponent *transform = sceneGetComponentFromEntity(scene, entityID, transID);
	transform->position[0] += 1.0f;
}

void nameSystem(Scene *scene, UUID entityID)
{
	UUID nameID = {};
	strcpy(nameID.string, "name");
	NameComponent *name = sceneGetComponentFromEntity(scene, entityID, nameID);
	printf("%s\n", name->name);
}

int32 equint32(void *num1, void *num2)
{
	return *(uint32 *)num1 == *(uint32 *)num2;
}

int main()
{
	// TODO: Test hash table
	HashMap map = createHashMap(sizeof(uint32), sizeof(uint32), 7, &equint32);

	UUID k = {};
	uint32 v = 5;

	memset(k.string, 0, sizeof(UUID));
	strcpy(k.string, "transform");
	v = 1;
	hashMapInsert(map, &k, &v);
	ASSERT(hashMapGetKey(map, &k));
	ASSERT(*(uint32 *)hashMapGetKey(map, &k) == 1);

	memset(k.string, 0, sizeof(UUID));
	strcpy(k.string, "name");
	v = 2;
	hashMapInsert(map, &k, &v);
	ASSERT(hashMapGetKey(map, &k));
	ASSERT(*(uint32 *)hashMapGetKey(map, &k) == 2);

	memset(k.string, 0, sizeof(UUID));
	strcpy(k.string, "mesh");
	v = 3;
	hashMapInsert(map, &k, &v);
	ASSERT(hashMapGetKey(map, &k));
	ASSERT(*(uint32 *)hashMapGetKey(map, &k) == 3);

	memset(k.string, 0, sizeof(UUID));
	strcpy(k.string, "transform");
	ASSERT(hashMapGetKey(map, &k));
	ASSERT(*(uint32 *)hashMapGetKey(map, &k) == 1);

	freeHashMap(&map);

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

	// TODO: Setup basic component state
	Scene *scene = createScene();

	// Add component types
	UUID transformComponentID = {};
	strcpy(transformComponentID.string, "transform");
	sceneAddComponentType(scene, transformComponentID, sizeof(TransformComponent), 2);

	UUID nameComponentID = {};
	strcpy(nameComponentID.string, "name");
	sceneAddComponentType(scene, nameComponentID, sizeof(NameComponent), 2);

	// Add systems
	List movementComponents = createList(sizeof(UUID));
	listPushFront(&movementComponents, &transformComponentID);
	System movementSystem = createSystem(movementComponents, 0, &moveSystem, 0);

	List nameComponents = createList(sizeof(UUID));
	listPushFront(&nameComponents, &nameComponentID);
	System printNameSystem = createSystem(movementComponents, 0, &nameSystem, 0);

	// Create entities
	UUID entity1 = sceneCreateEntity(scene);
	UUID entity2 = sceneCreateEntity(scene);

	TransformComponent transform = {};
	transform.position[0] = 1.0f;
	transform.position[1] = 1.0f;
	transform.position[2] = 1.0f;
	sceneAddComponentToEntity(scene, entity1, transformComponentID, &transform);

	NameComponent name = {};
	strcpy(name.name, "Hello, world!");
	sceneAddComponentToEntity(scene, entity2, nameComponentID, &name);

	// State previous
	// State next

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
			systemRun(scene, &movementSystem);
			systemRun(scene, &printNameSystem);

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

	sceneRemoveComponentType(scene, transformComponentID);
	sceneRemoveComponentType(scene, nameComponentID);

	freeScene(&scene);
	freeWindow(window);

	return 0;
}
