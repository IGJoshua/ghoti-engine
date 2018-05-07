#pragma once
#include "defines.h"

#include "data/data_types.h"

typedef union uuid_t {
	char string[64];
	uint8 bytes[64];
} UUID;

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
} Scene;
