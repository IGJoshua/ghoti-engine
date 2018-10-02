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
int32 reloadScene(const char *name, bool reloadAssets);
int32 reloadAllScenes(bool reloadAssets);
int32 unloadScene(const char *name);

List activeScenes;

void sceneRegisterEntity(Scene *s, UUID newEntity);
UUID sceneCreateEntity(Scene *s);
void sceneRemoveEntity(Scene *s, UUID entity);

int32 sceneAddComponentToEntity(
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

void exportSceneSnapshot(Scene *scene, const char *filename);
void exportEntitySnapshot(
	Scene *scene,
	UUID entity,
	const char *filename);

]]
