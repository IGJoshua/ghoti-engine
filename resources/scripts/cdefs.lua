ffi.cdef[[
typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef double real64;
typedef float real32;

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

typedef struct list_iterator_t
{
  ListNode *prev;
  ListNode *curr;
} ListIterator;


ListIterator listGetIterator(List *l);
void listMoveIterator(ListIterator *itr);
int32 listIteratorAtEnd(ListIterator itr);

void listPushFront(List *l, void *data);
void listPushBack(List *l, void *data);

void listPopFront(List *l);
void listClear(List *l);

void listRemove(List *l, ListIterator *itr);
void listInsert(List *l, ListIterator *itr, void *data);

typedef List HashMapBucket;

typedef int32(*ComparisonOp)(void*, void*);

typedef struct hash_map_t
{
  uint32 keySizeBytes;
  uint32 valueSizeBytes;
  uint32 bucketCount;
  ComparisonOp comparison;
  uint32 count;
  HashMapBucket buckets[];
} *HashMap;

void *hashMapGetData(HashMap map, void *key);

typedef union uuid_t
{
  char string[64];
  uint8 bytes[64];
} UUID;

UUID idFromName(const char *name);
UUID stringToUUID(const char *string);

void setMousePosition(real64 x, real64 y);
void setMouseHidden(bool hidden);
void setMouseLocked(bool locked);

bool getVSYNCMode(void);
void switchVSYNCMode(void);
void setVSYNCMode(bool vsync);

bool getFullscreenMode(void);
void switchFullscreenMode(void);
void setFullscreenMode(bool fullscreen);

void getViewportSize(int32 *width, int32 *height);

int32 closeWindow();

typedef enum data_type_e {
	INVALID_DATA_TYPE = -1,
	DATA_TYPE_UINT8 = 0,
	DATA_TYPE_UINT16,
	DATA_TYPE_UINT32,
	DATA_TYPE_UINT64,
	DATA_TYPE_INT8,
	DATA_TYPE_INT16,
	DATA_TYPE_INT32,
	DATA_TYPE_INT64,
	DATA_TYPE_FLOAT32,
	DATA_TYPE_FLOAT64,
	DATA_TYPE_BOOL,
	DATA_TYPE_CHAR,
	DATA_TYPE_STRING,
	DATA_TYPE_UUID,
	DATA_TYPE_ENUM,
	DATA_TYPE_PTR
} DataType;

int32 strcmp(const char *str1, const char *str2);


]]

require("resources/scripts/cdefs/kazmath")
require("resources/scripts/cdefs/math")
require("resources/scripts/cdefs/componentDefs")
require("resources/scripts/cdefs/sceneDefs")
require("resources/scripts/cdefs/GLFW")
require("resources/scripts/cdefs/gui")
require("resources/scripts/cdefs/jsonUtil")
require("resources/scripts/cdefs/saving")
require("resources/scripts/cdefs/physics")
require("resources/scripts/cdefs/assetManagement")
require("resources/scripts/cdefs/audio")