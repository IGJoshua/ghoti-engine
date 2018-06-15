ffi.cdef[[

typedef struct scene_t
{
  HashMap componentTypes;
  HashMap entities;
  UUID mainCamera;
  List physicsFrameSystems;
  List renderFrameSystems;
  List luaPhysicsFrameSystemNames;
  List luaRenderFrameSystemNames;
} Scene;

Scene *createScene(void);
int32 loadScene(const char *name, Scene **scene);
void freeScene(Scene **scene);

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
