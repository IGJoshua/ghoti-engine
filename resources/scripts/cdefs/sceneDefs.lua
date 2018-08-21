ffi.cdef[[

typedef struct scene_t
{
	char *name;
	HashMap componentTypes;
	HashMap entities;
	UUID mainCamera;
	UUID player;
	List physicsFrameSystems;
	List renderFrameSystems;
	List luaPhysicsFrameSystemNames;
	List luaRenderFrameSystemNames;
	uint32 numComponentLimitNames;
	char **componentLimitNames;
	HashMap componentDefinitions;
	bool loadedThisFrame;
	void *physicsWorld;
	void *physicsSpace;
	void *contactGroup;
	real32 gravity;
} Scene;

Scene *createScene(void);
int32 loadScene(const char *name);
int32 reloadScene(const char *name);
int32 reloadAllScenes(void);
int32 unloadScene(const char *name);

List activeScenes;
bool changeScene;
bool reloadingScene;
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
