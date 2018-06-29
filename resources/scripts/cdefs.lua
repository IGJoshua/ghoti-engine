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
  HashMapBucket buckets[];
} *HashMap;

void *hashMapGetKey(HashMap map, void *key);

typedef union uuid_t
{
  char string[64];
  uint8 bytes[64];
} UUID;

UUID idFromName(const char *name);

int32 closeWindow();

typedef enum data_type_e {
	INVALID_DATA_TYPE = -1,
	UINT8 = 0,
	UINT16,
	UINT32,
	UINT64,
	INT8,
	INT16,
	INT32,
	INT64,
	FLOAT32,
	FLOAT64,
	BOOL,
	CHAR,
	STRING
} DataType;

int32 strcmp(const char *str1, const char *str2);

]]

local kazmath = require("resources/scripts/cdefs/kazmath")
local components = require("resources/scripts/cdefs/componentDefs")
local sceneDefs = require("resources/scripts/cdefs/sceneDefs")
local glfwKeys = require("resources/scripts/cdefs/GLFW")
local jsonUtil = require("resources/scripts/cdefs/jsonUtil")
local saving = require("resources/scripts/cdefs/saving")
