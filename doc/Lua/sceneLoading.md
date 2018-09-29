### [Table of Contents](../main.md) -> [Lua](Lua.md) -> Scene Loading

#Scene Loading
A structure that represents a scene and all of the entities in it.
```c
typedef struct scene_t
{
	char *name;
	// Maps component UUIDs to pointers to component data tables
	HashMap componentTypes;
	// Maps entity UUIDs to lists of component UUIDs
	HashMap entities;
	UUID mainCamera;
	UUID player;
	List physicsFrameSystems;
	List renderFrameSystems;
	List luaPhysicsFrameSystemNames;
	List luaRenderFrameSystemNames;
	uint32 numComponentLimitNames;
	char **componentLimitNames;
	// Maps component UUIDs to component definitions
	HashMap componentDefinitions;
	bool loadedThisFrame;
	dWorldID physicsWorld;
	dSpaceID physicsSpace;
	dJointGroupID contactGroup;
	real32 gravity;
} Scene;
```

## Scene *createScene
Creates a scene
```c
Scene *createScene(void);
```

## int32 loadScene
Loads a scene by name
```c
int32 loadScene(const char *name);
```

## void freeScene
Frees a scene pointer for later use or deletion
```c
void freeScene(Scene **scene);
```
## UUID sceneCreateEntity
Creates an entity, giving it a `UUID`
```c
UUID sceneCreateEntity(Scene *s);
```
## void sceneRegisterEntity
Registers an entity in a scene using its `UUID`
```c
void sceneRegisterEntity(Scene *s, UUID newEntity);
```
## void sceneRemoveEntity
Removes an entity in a scene using its `UUID`
```c
void sceneRemoveEntity(Scene *s, UUID entity);
```
## void sceneAddComponentToEntity
Adds a component to an entity. Requires a pointer to the scene, the UUID of the entity, the UUID of the component's type, and a pointer to the data of the component
```c
void sceneAddComponentToEntity(
  Scene *s,
  UUID entity,
  UUID componentType,
  void *componentData);
```
## void sceneRemoveComponentFromEntity
Deletes component data from an entity.
```c
void sceneRemoveComponentFromEntity(
  Scene *s,
  UUID entity,
  UUID componentType);
```
## void *sceneGetComponentFromEntity
Gets the component from an entity through a void pointer
```c
void *sceneGetComponentFromEntity(
  Scene *s,
  UUID entity,
  UUID componentType);
```
## void sceneAddComponentType
Adds a component type to the scene.
```c
void sceneAddComponentType(
  Scene *scene,
  UUID componentID,
  uint32 componentSize,
  uint32 maxComponents);
```
