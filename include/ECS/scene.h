#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define COMPONENT_TYPE_BUCKETS 97
#define ENTITY_BUCKETS 97

Scene *createScene();
void freeScene(Scene **scene);

UUID sceneCreateEntity(Scene *s);
void sceneRemoveEntity(Scene *s, UUID entity);

void sceneAddComponentToEntity(
	Scene *s,
	UUID entity,
	UUID componentType,
	void *componentData
);

void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType
);

void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType
);
