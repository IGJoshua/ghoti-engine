#pragma once
#include "defines.h"

typedef union uuid_t {
	char string[64];
	uint8 bytes[64];
} UUID;

typedef struct component_data_table_t
{
	uint64 size;
	uint32 numEntries;
	uint8 data[];
} ComponentDataTable;

typedef struct component_type_entry_t
{
	char componentName[64];
	ComponentDataTable *dataTable;
} ComponentTypeEntry;

typedef struct component_type_table_t
{
	uint32 count;
	ComponentTypeEntry entries[];
} ComponentTypeTable;

typedef struct scene_t
{
	ComponentTypeTable *componentTypes;
	// TODO: Entity map, maps UUID's to lists of component type id's
} Scene;
