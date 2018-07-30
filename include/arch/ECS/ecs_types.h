#pragma once
#include "defines.h"

#include "data/data_types.h"

#include <ode/ode.h>

#define UUID_LENGTH 63

typedef union uuid_t
{
	char string[UUID_LENGTH + 1];
	uint8 bytes[UUID_LENGTH + 1];
} UUID;

typedef struct component_data_table_t
{
	// Maximum number of active components
	uint32 numEntries;
	// Byte size of the component structure
	uint32 componentSize;
	// Maps UUID to uint32 index of component
	HashMap idToIndex;
	// Data table
	uint8 data[];
} ComponentDataTable;

typedef struct
{
	uint32 index;
	ComponentDataTable *table;
} ComponentDataTableIterator;

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
	DATA_TYPE_UUID
} DataType;

typedef struct component_value_definition_t
{
	char *name;
	DataType type;
	uint32 maxStringSize;
	uint32 count;
} ComponentValueDefinition;

typedef struct component_definition_t
{
	char *name;
	uint32 size;
	uint32 numValues;
	ComponentValueDefinition *values;
} ComponentDefinition;

typedef struct scene_t
{
	char *name;
	// Maps component UUIDs to pointers to component data tables
	HashMap componentTypes;
	// Maps entity UUIDs to lists of component UUIDs
	HashMap entities;
	UUID mainCamera;
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

typedef void(*InitSystem)(Scene *scene);
typedef void(*BeginSystem)(Scene *scene, real64 dt);
typedef void(*RunSystem)(Scene *scene, UUID entityID, real64 dt);
typedef void(*EndSystem)(Scene *scene, real64 dt);
typedef void(*ShutdownSystem)(Scene *scene);

typedef struct system_t
{
	List componentTypes;

	InitSystem init;
	BeginSystem begin;
	RunSystem run;
	EndSystem end;
	ShutdownSystem shutdown;
} System;
