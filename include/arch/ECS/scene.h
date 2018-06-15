#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <luajit-2.0/lua.h>

#define COMPONENT_TYPE_BUCKETS 97
#define ENTITY_BUCKETS 97

Scene *createScene(void);
int32 loadScene(const char *name, Scene **scene);
int32 loadSceneEntities(Scene **scene, const char *name, bool loadData);
void freeScene(Scene **scene);

ComponentDefinition getComponentDefinition(const Scene *scene, UUID name);
void copyComponentDefinition(
	ComponentDefinition *dest,
	ComponentDefinition *src);
void freeComponentDefinition(ComponentDefinition *componentDefinition);

uint32 getDataTypeSize(DataType type);
char* getDataTypeString(
	const ComponentValueDefinition *componentValueDefinition);

void exportEntity(const Scene *scene, UUID entity, const char *filename);
void exportScene(const Scene *scene, const char *filename);
void exportSave(void *data, uint32 size, const Scene *scene, uint32 slot);

void sceneAddRenderFrameSystem(
	Scene *scene,
	System system);
void sceneAddPhysicsFrameSystem(
	Scene *scene,
	System system);

void sceneInitRenderFrameSystems(Scene *scene);
void sceneInitPhysicsFrameSystems(Scene *scene);
void sceneInitSystems(Scene *scene);

void sceneRunRenderFrameSystems(Scene *scene, real64 dt);
void sceneRunPhysicsFrameSystems(Scene *scene, real64 dt);

void sceneShutdownRenderFrameSystems(Scene *scene);
void sceneShutdownPhysicsFrameSystems(Scene *scene);
void sceneShutdownSystems(Scene *scene);

void sceneInitLua(lua_State **L, Scene *scene);
void sceneShutdownLua(lua_State **L, Scene *scene);

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
