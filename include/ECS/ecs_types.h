#pragma once
#include "defines.h"

#include "data/data_types.h"

typedef union uuid_t {
	char string[64];
	uint8 bytes[64];
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

typedef struct scene_t
{
	// Maps component UUIDs to pointers to component data tables
	HashMap componentTypes;
	// Maps entity UUIDs to lists of component UUIDs
	HashMap entities;
	UUID mainCamera;
} Scene;

typedef void(*InitSystem)(Scene *scene);
typedef void(*SystemFn)(Scene *scene, UUID entityID);
typedef void(*ShutdownSystem)(Scene *scene);

typedef struct system_t
{
	List componentTypes;

	InitSystem init;
	SystemFn fn;
	ShutdownSystem shutdown;
} System;
