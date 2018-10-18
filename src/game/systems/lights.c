#include "defines.h"

#include "core/config.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/light.h"

internal UUID transformComponentID = {};
internal UUID lightComponentID = {};

internal uint32 lightsSystemRefCount = 0;

extern Config config;

uint32 numDirectionalLights = 0;
DirectionalLight directionalLight;

uint32 numPointLights = 0;
PointLight pointLights[MAX_NUM_POINT_LIGHTS];

uint32 numSpotlights = 0;
Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

internal TransformComponent* shadowPointLightTransforms
	[MAX_NUM_SHADOW_POINT_LIGHTS];

internal TransformComponent* shadowSpotlightTransforms
	[MAX_NUM_SHADOW_SPOTLIGHTS];

extern uint32 numShadowDirectionalLights;
extern ShadowDirectionalLight shadowDirectionalLight;

extern uint32 numShadowPointLights;
extern ShadowPointLight shadowPointLights[MAX_NUM_SHADOW_POINT_LIGHTS];

extern uint32 numShadowSpotlights;
extern ShadowSpotlight shadowSpotlights[MAX_NUM_SHADOW_SPOTLIGHTS];

extern uint32 shadowsSystemRefCount;

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

	if (shadowsSystemRefCount == 0)
	{
		return;
	}

	TransformComponent *cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		transformComponentID);

	if (!cameraTransform)
	{
		return;
	}

	real32 closestPointLightDistances[MAX_NUM_SHADOW_POINT_LIGHTS];

	for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
	{
		closestPointLightDistances[i] = FLT_MAX;
	}

	real32 closestSpotlightDistances[MAX_NUM_SHADOW_SPOTLIGHTS];

	for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
	{
		closestSpotlightDistances[i] = FLT_MAX;
	}

	ComponentDataTable *lightComponents =
		*(ComponentDataTable**)hashMapGetData(
			scene->componentTypes,
			&lightComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(lightComponents);
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		LightComponent *lightComponent = cdtIteratorGetData(itr);
		if (lightComponent->enabled)
		{
			TransformComponent *transform = sceneGetComponentFromEntity(
				scene,
				cdtIteratorGetUUID(itr),
				transformComponentID);

			kmVec3 displacement;
			kmVec3Subtract(
				&displacement,
				&transform->globalPosition,
				&cameraTransform->globalPosition);

			real32 distance = kmVec3LengthSq(&displacement);
			int32 index = -1;

			switch (lightComponent->type)
			{
				case LIGHT_TYPE_POINT:
					for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
					{
						if (!shadowPointLightTransforms[i])
						{
							index = i;
							break;
						}
					}

					if (index == -1)
					{
						for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
						{
							if (distance < closestPointLightDistances[i])
							{
								index = i;
								break;
							}
						}
					}

					if (index > -1)
					{
						closestPointLightDistances[index] = distance;
						shadowPointLightTransforms[index] = transform;
					}

					break;
				case LIGHT_TYPE_SPOT:
					for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
					{
						if (!shadowSpotlightTransforms[i])
						{
							index = i;
							break;
						}
					}

					if (index == -1)
					{
						for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
						{
							if (distance < closestSpotlightDistances[i])
							{
								index = i;
								break;
							}
						}
					}

					if (index > -1)
					{
						closestSpotlightDistances[index] = distance;
						shadowSpotlightTransforms[index] = transform;
					}

					break;
				default:
					break;
			}
		}
	}

	uint32 numOverflowShadowPointLights =
		MAX_NUM_SHADOW_POINT_LIGHTS -
			config.graphicsConfig.maxNumShadowPointLights;

	memset(
		&shadowPointLightTransforms[
			config.graphicsConfig.maxNumShadowPointLights],
		0,
		numOverflowShadowPointLights * sizeof(TransformComponent*));

	uint32 numOverflowShadowSpotlights =
		MAX_NUM_SHADOW_SPOTLIGHTS -
			config.graphicsConfig.maxNumShadowSpotlights;

	memset(
		&shadowSpotlightTransforms[
			config.graphicsConfig.maxNumShadowSpotlights],
		0,
		numOverflowShadowSpotlights * sizeof(TransformComponent*));
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

	for (uint32 i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
	{
		pointLights[i].shadowIndex = -1;
	}

	numSpotlights = 0;
	memset(spotlights, 0, MAX_NUM_SPOTLIGHTS * sizeof(Spotlight));

	for (uint32 i = 0; i < MAX_NUM_SPOTLIGHTS; i++)
	{
		spotlights[i].shadowIndex = -1;
	}

	numShadowDirectionalLights = 0;
	numShadowPointLights = 0;
	numShadowSpotlights = 0;

	memset(
		shadowPointLightTransforms,
		0,
		MAX_NUM_SHADOW_POINT_LIGHTS * sizeof(TransformComponent*));
	memset(
		shadowSpotlightTransforms,
		0,
		MAX_NUM_SHADOW_SPOTLIGHTS * sizeof(TransformComponent*));
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

	kmVec3Assign(&directionalLight.radiantFlux, &light->radiantFlux);

	if (!config.graphicsConfig.pbr)
	{
		directionalLight.radiantFlux.x = kmClamp(
			directionalLight.radiantFlux.x,
			0.0f,
			1.0f);
		directionalLight.radiantFlux.y = kmClamp(
			directionalLight.radiantFlux.y,
			0.0f,
			1.0f);
		directionalLight.radiantFlux.z = kmClamp(
			directionalLight.radiantFlux.z,
			0.0f,
			1.0f);
	}

	kmQuaternionAssign(
		&directionalLight.previousDirection,
		&transform->lastGlobalRotation);
	kmQuaternionAssign(&directionalLight.direction, &transform->globalRotation);

	if (shadowsSystemRefCount == 0 ||
		!config.graphicsConfig.directionalLightShadows ||
		numShadowDirectionalLights == 1)
	{
		return;
	}

	numShadowDirectionalLights++;

	kmQuaternionAssign(
		&shadowDirectionalLight.previousDirection,
		&transform->lastGlobalRotation);
	kmQuaternionAssign(
		&shadowDirectionalLight.direction,
		&transform->globalRotation);
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

	kmVec3Assign(&pointLight->radiantFlux, &light->radiantFlux);

	if (!config.graphicsConfig.pbr)
	{
		pointLight->radiantFlux.x = kmClamp(
			pointLight->radiantFlux.x,
			0.0f,
			1.0f);
		pointLight->radiantFlux.y = kmClamp(
			pointLight->radiantFlux.y,
			0.0f,
			1.0f);
		pointLight->radiantFlux.z = kmClamp(
			pointLight->radiantFlux.z,
			0.0f,
			1.0f);
	}

	kmVec3Assign(&pointLight->previousPosition, &transform->lastGlobalPosition);
	kmVec3Assign(&pointLight->position, &transform->globalPosition);

	pointLight->radius = light->radius;

	if (shadowsSystemRefCount == 0 ||
		numShadowPointLights == config.graphicsConfig.maxNumShadowPointLights)
	{
		return;
	}

	for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
	{
		if (transform == shadowPointLightTransforms[i])
		{
			pointLight->shadowIndex = numShadowPointLights++;

			ShadowPointLight *shadowPointLight = &shadowPointLights[i];

			kmVec3Assign(
				&shadowPointLight->previousPosition,
				&transform->lastGlobalPosition);
			kmVec3Assign(
				&shadowPointLight->position,
				&transform->globalPosition);

			shadowPointLight->farPlane = light->radius;

			break;
		}
	}
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

	kmVec3Assign(&spotlight->radiantFlux, &light->radiantFlux);

	if (!config.graphicsConfig.pbr)
	{
		spotlight->radiantFlux.x = kmClamp(
			spotlight->radiantFlux.x,
			0.0f,
			1.0f);
		spotlight->radiantFlux.y = kmClamp(
			spotlight->radiantFlux.y,
			0.0f,
			1.0f);
		spotlight->radiantFlux.z = kmClamp(
			spotlight->radiantFlux.z,
			0.0f,
			1.0f);
	}

	kmVec3Assign(&spotlight->previousPosition, &transform->lastGlobalPosition);
	kmVec3Assign(&spotlight->position, &transform->globalPosition);

	kmQuaternionAssign(
		&spotlight->previousDirection,
		&transform->lastGlobalRotation);
	kmQuaternionAssign(&spotlight->direction, &transform->globalRotation);

	spotlight->radius = light->radius;
	kmVec2Assign(&spotlight->size, &light->size);

	if (shadowsSystemRefCount == 0 ||
		numShadowSpotlights == config.graphicsConfig.maxNumShadowSpotlights)
	{
		return;
	}

	for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
	{
		if (transform == shadowSpotlightTransforms[i])
		{
			spotlight->shadowIndex = numShadowSpotlights++;

			ShadowSpotlight *shadowSpotlight = &shadowSpotlights[i];

			kmVec3Assign(
				&shadowSpotlight->previousPosition,
				&transform->lastGlobalPosition);
			kmVec3Assign(
				&shadowSpotlight->position,
				&transform->globalPosition);

			kmQuaternionAssign(
				&shadowSpotlight->previousDirection,
				&transform->lastGlobalRotation);
			kmQuaternionAssign(
				&shadowSpotlight->direction,
				&transform->globalRotation);

			shadowSpotlight->fov =
				2.0f * acosf(spotlight->size.y) * (180.0f / M_PI);
			shadowSpotlight->farPlane = spotlight->radius;

			break;
		}
	}
}