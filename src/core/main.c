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

void moveSystem(Scene *scene, UUID entityID)
{
	puts("Move system.");
	printf("Entity ID: %s\n", entityID.string);

	UUID transID = {};
	strcpy(transID.string, "transform");
	TransformComponent *transform = sceneGetComponentFromEntity(scene, entityID, transID);

	printf("New Location: %f\n", transform->position.x);

	transform->position.x = sinf(glfwGetTime() * 3);
	transform->position.y = cosf(glfwGetTime() * 3);
}

void nameSystem(Scene *scene, UUID entityID)
{
	puts("Name system.");
	printf("Entity ID: %s\n", entityID.string);

	UUID nameID = {};
	strcpy(nameID.string, "name");
	NameComponent *name = sceneGetComponentFromEntity(scene, entityID, nameID);
	printf("%s\n", name->name);
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
	real64 dt = 1.0 / 60.0;

	real64 currentTime = glfwGetTime();
	real64 accumulator = 0.0;

	// TODO: Setup basic component state
	Scene *scene = createScene();

	// Add component types
	UUID transformComponentID = {};
	strcpy(transformComponentID.string, "transform");
	sceneAddComponentType(scene, transformComponentID, sizeof(TransformComponent), 4);

	UUID nameComponentID = {};
	strcpy(nameComponentID.string, "name");
	sceneAddComponentType(scene, nameComponentID, sizeof(NameComponent), 4);

	UUID modelComponentID = {};
	strcpy(modelComponentID.string, "model");
	sceneAddComponentType(scene, modelComponentID, sizeof(ModelComponent), 4);

	// Add systems
	List movementComponents = createList(sizeof(UUID));
	listPushFront(&movementComponents, &transformComponentID);
	System movementSystem = createSystem(movementComponents, 0, &moveSystem, 0);

	List nameComponents = createList(sizeof(UUID));
	listPushFront(&nameComponents, &nameComponentID);
	System printNameSystem = createSystem(nameComponents, 0, &nameSystem, 0);

	System rendererSystem = createRendererSystem();

	// Create entities
	UUID entity1 = {};
	strcpy(entity1.string, "ENTITY1");
	sceneRegisterEntity(scene, entity1);
	UUID entity2 = {};
	strcpy(entity2.string, "ENTITY2");
	sceneRegisterEntity(scene, entity2);

	UUID teapot = {};
	strcpy(teapot.string, "TEAPOT");
	sceneRegisterEntity(scene, teapot);

	UUID test = {};
	strcpy(test.string, "test");
	sceneRegisterEntity(scene, test);

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
	kmVec3Zero(&transform.position);
	transform.scale.x = 0.01f;
	transform.scale.y = 0.01f;
	transform.scale.z = 0.01f;
	kmQuaternionRotationPitchYawRoll(&transform.rotation, kmDegreesToRadians(90), 0, 0);
	sceneAddComponentToEntity(scene, teapot, transformComponentID, &transform);

	ModelComponent testModel = {};
	strcpy(testModel.name, "test");
	sceneAddComponentToEntity(scene, test, modelComponentID, &testModel);
	kmVec3Fill(&transform.position, -1, 0, 0);
	transform.scale.x = 1;
	transform.scale.y = 1;
	transform.scale.z = 1;
	kmQuaternionRotationPitchYawRoll(&transform.rotation, kmDegreesToRadians(90), 0, 0);
	sceneAddComponentToEntity(scene, test, transformComponentID, &transform);

	// State previous
	// State next

	// TODO: Make this thing work
  	//loadScene("scene_1", &scene);

	rendererSystem.init(scene);

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

		int32 width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		//real32 aspectRatio = (real32)width / (real32)height;

		// Render
		systemRun(scene, &rendererSystem);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	freeModel("teapot");
	freeModel("test");

	sceneRemoveEntity(scene, entity1);
	sceneRemoveEntity(scene, entity2);
	sceneRemoveEntity(scene, teapot);
	sceneRemoveEntity(scene, test);

	sceneRemoveComponentType(scene, transformComponentID);
	sceneRemoveComponentType(scene, nameComponentID);
	sceneRemoveComponentType(scene, modelComponentID);

	listClear(&nameComponents);
	listClear(&movementComponents);
	freeRendererSystem(&rendererSystem);

	freeScene(&scene);
	freeWindow(window);

	return 0;
}
