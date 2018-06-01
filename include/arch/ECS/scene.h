#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define COMPONENT_TYPE_BUCKETS 97
#define ENTITY_BUCKETS 97

Scene *createScene(void);
void freeScene(Scene **scene);

void sceneAddRenderFrameSystem(
	Scene *scene,
	System system);
void sceneAddPhysicsFrameSystem(
	Scene *scene,
	System system);

void sceneInitRenderFrameSystems(Scene *scene);
void sceneInitPhysicsFrameSystems(Scene *scene);
void sceneInitSystems(Scene *scene);

void sceneRunRenderFrameSystems(Scene *scene);
void sceneRunPhysicsFrameSystems(Scene *scene);

void sceneShutdownRenderFrameSystems(Scene *scene);
void sceneShutdownPhysicsFrameSystems(Scene *scene);
void sceneShutdownSystems(Scene *scene);

void sceneAddComponentType(
	Scene *scene,
	UUID componentID,
	uint32 componentSize,
	uint32 maxComponents);
void sceneRemoveComponentType(Scene *scene, UUID componentID);

void sceneRegisterEntity(Scene *s, UUID newEntity);
UUID sceneCreateEntity(Scene *s);
void sceneRemoveEntity(Scene *s, UUID entity);

void sceneAddComponentToEntity(
	Scene *s,
	UUID entity,
	UUID componentType,
	void *componentData);
void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType);
void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType);

UUID idFromName(const char *name);
