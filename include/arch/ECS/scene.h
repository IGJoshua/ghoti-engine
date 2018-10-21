#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <luajit-2.0/lua.h>

#define COMPONENT_TYPE_BUCKETS 97
#define ENTITY_BUCKETS 50021
#define COMPONENT_DEFINITION_BUCKETS 97

#define RUNTIME_STATE_DIR "resources/.runtime-state"

Scene *createScene(void);
int32 loadSceneFile(const char *name, Scene **scene);
Scene *getScene(const char *name);
void freeScene(Scene **scene);

int32 loadScene(const char *name);
int32 reloadScene(const char *name, bool reloadAssets, bool togglePBR);
int32 reloadAllScenes(bool reloadAssets, bool togglePBR);
int32 unloadScene(const char *name);
int32 shutdownScene(Scene **scene);
int32 deactivateScene(Scene *scene);

void exportEntitySnapshot(
	Scene *scene,
	UUID entity,
	const char *filename);
void exportSceneSnapshot(Scene *scene, const char *filename);

void sceneAddRenderFrameSystem(
	Scene *scene,
	UUID system);
void sceneAddPhysicsFrameSystem(
	Scene *scene,
	UUID system);

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
void sceneRemoveEntityComponents(Scene *s, UUID entity);

int32 sceneAddComponentToEntity(
	Scene *s,
	UUID entity,
	UUID componentType,
	void *componentData);
void sceneRemoveComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType);
void sceneRemoveComponentFromAllEntities(Scene *scene, UUID componentID);
void *sceneGetComponentFromEntity(
	Scene *s,
	UUID entity,
	UUID componentType);

UUID idFromName(const char *name);
