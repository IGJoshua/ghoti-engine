#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"
#include "components/light.h"

internal UUID transformComponentID = {};
internal UUID lightComponentID = {};

internal uint32 lightsSystemRefCount = 0;

uint32 numDirectionalLights = 0;
DirectionalLight directionalLight;

uint32 numPointLights = 0;
PointLight pointLights[MAX_NUM_POINT_LIGHTS];

uint32 numSpotlights = 0;
Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

internal void clearLights(void);

internal void addDirectionalLight(
	TransformComponent *transform,
	LightComponent *light);
internal void addPointLight(
	TransformComponent *transform,
	LightComponent *light);
internal void addSpotlight(
	TransformComponent *transform,
	LightComponent *light);

internal void initLightsSystem(Scene *scene)
{
	lightsSystemRefCount++;
}

internal void beginLightsSystem(Scene *scene, real64 dt)
{
	clearLights();
}

internal void runLightsSystem(Scene *scene, UUID entity, real64 dt)
{
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entity,
		transformComponentID);
	LightComponent *light = sceneGetComponentFromEntity(
		scene,
		entity,
		lightComponentID);

	switch (light->type)
	{
		case LIGHT_TYPE_DIRECTIONAL:
			addDirectionalLight(transform, light);
			break;
		case LIGHT_TYPE_POINT:
			addPointLight(transform, light);
			break;
		case LIGHT_TYPE_SPOT:
			addSpotlight(transform, light);
			break;
		default:
			break;
	}
}

internal void shutdownLightsSystem(Scene *scene)
{
	if (--lightsSystemRefCount == 0)
	{
		clearLights();
	}
}

System createLightsSystem(void)
{
	transformComponentID = idFromName("transform");
	lightComponentID = idFromName("light");

	System system = {};

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &lightComponentID);

	system.init = &initLightsSystem;
	system.begin = &beginLightsSystem;
	system.run = &runLightsSystem;
	system.shutdown = &shutdownLightsSystem;

	return system;
}

void clearLights(void)
{
	numDirectionalLights = 0;
	memset(&directionalLight, 0, sizeof(DirectionalLight));

	numPointLights = 0;
	memset(pointLights, 0, MAX_NUM_POINT_LIGHTS * sizeof(PointLight));

	numSpotlights = 0;
	memset(spotlights, 0, MAX_NUM_SPOTLIGHTS * sizeof(Spotlight));
}

void addDirectionalLight(
	TransformComponent *transform,
	LightComponent *light)
{
	if (!light->enabled || numDirectionalLights == 1)
	{
		return;
	}

	numDirectionalLights++;

	kmVec3Assign(&directionalLight.color, &light->color);
	kmVec3Assign(&directionalLight.ambient, &light->ambient);

	kmQuaternionAssign(
		&directionalLight.previousDirection,
		&transform->lastGlobalRotation);
	kmQuaternionAssign(&directionalLight.direction, &transform->globalRotation);
}

void addPointLight(
	TransformComponent *transform,
	LightComponent *light)
{
	if (!light->enabled || numPointLights == MAX_NUM_POINT_LIGHTS)
	{
		return;
	}

	PointLight *pointLight = &pointLights[numPointLights++];

	kmVec3Assign(&pointLight->color, &light->color);
	kmVec3Assign(&pointLight->ambient, &light->ambient);

	kmVec3Assign(&pointLight->previousPosition, &transform->lastGlobalPosition);
	kmVec3Assign(&pointLight->position, &transform->globalPosition);

	pointLight->constantAttenuation = light->constantAttenuation;
	pointLight->linearAttenuation = light->linearAttenuation;
	pointLight->quadraticAttenuation = light->quadraticAttenuation;
}

void addSpotlight(
	TransformComponent *transform,
	LightComponent *light)
{
	if (!light->enabled || numSpotlights == MAX_NUM_SPOTLIGHTS)
	{
		return;
	}

	Spotlight *spotlight = &spotlights[numSpotlights++];

	kmVec3Assign(&spotlight->color, &light->color);
	kmVec3Assign(&spotlight->ambient, &light->ambient);

	kmVec3Assign(&spotlight->previousPosition, &transform->lastGlobalPosition);
	kmVec3Assign(&spotlight->position, &transform->globalPosition);

	kmQuaternionAssign(
		&spotlight->previousDirection,
		&transform->lastGlobalRotation);
	kmQuaternionAssign(&spotlight->direction, &transform->globalRotation);

	spotlight->constantAttenuation = light->constantAttenuation;
	spotlight->linearAttenuation = light->linearAttenuation;
	spotlight->quadraticAttenuation = light->quadraticAttenuation;

	kmVec2Assign(&spotlight->size, &light->size);
}