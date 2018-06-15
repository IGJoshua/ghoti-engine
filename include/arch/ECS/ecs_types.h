#pragma once
#include "defines.h"

#include "data/data_types.h"

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
	// Maps component UUIDs to pointers to component data tables
	HashMap componentTypes;
	// Maps entity UUIDs to lists of component UUIDs
	HashMap entities;
	UUID mainCamera;
	List physicsFrameSystems;
	List renderFrameSystems;
	List luaPhysicsFrameSystemNames;
	List luaRenderFrameSystemNames;
	uint32 numComponentsDefinitions;
	ComponentDefinition *componentDefinitions;
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
