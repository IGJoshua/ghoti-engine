ffi.cdef[[
typedef long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef double real64;
typedef float real32;

typedef enum { false, true } bool;

char *strcpy(char *destination, const char *source);

typedef struct list_node_t
{
  struct list_node_t *next;
  uint8 data[];
} ListNode;

typedef struct list_t
{
  uint32 dataSize;
  ListNode *front;
  ListNode *back;
} List;

typedef ListNode **ListIterator;

ListIterator listGetIterator(List *l);
void listMoveIterator(ListIterator *itr);
int32 listIteratorAtEnd(ListIterator itr);

void listRemove(List *l, ListIterator itr);
void listInsert(List *l, ListIterator itr, void *data);

typedef List HashMapBucket;

typedef int32(*ComparisonOp)(void*, void*);

typedef struct hash_map_t
{
  uint32 keySizeBytes;
  uint32 valueSizeBytes;
  uint32 bucketCount;
  ComparisonOp comparison;
  HashMapBucket buckets[];
} *HashMap;

void *hashMapGetKey(HashMap map, void *key);

typedef union uuid_t
{
  char string[64];
  uint8 bytes[64];
} UUID;

UUID idFromName(const char *name);

typedef struct component_data_table_t
{
  uint32 numEntries;
  uint32 componentSize;
  HashMap idToIndex;
  uint8 data[];
} ComponentDataTable;

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

local kazmath = require("kazmath")
