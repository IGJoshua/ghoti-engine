ffi.cdef[[

typedef struct scene_t
{
	char *name;
	HashMap componentTypes;
	HashMap entities;
	UUID mainCamera;
	List physicsFrameSystems;
	List renderFrameSystems;
	List luaPhysicsFrameSystemNames;
	List luaRenderFrameSystemNames;
	uint32 numComponentLimitNames;
	char **componentLimitNames;
	uint32 numComponentDefinitions;
	uint32 componentDefinitionsCapacity;
	ComponentDefinition *componentDefinitions;
} Scene;

Scene *createScene(void);
int32 loadScene(const char *name);
void freeScene(Scene **scene);


int32 reloadScene(const char *name);
int32 reloadAllScenes(void);
int32 unloadScene(const char *name);

List activeScenes;
uint32 changeScene;
List unloadedScenes;

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

void sceneAddComponentType(
  Scene *scene,
  UUID componentID,
  uint32 componentSize,
  uint32 maxComponents);

]]
