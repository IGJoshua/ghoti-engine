#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define CDT_ID_BUCKETS 97

ComponentDataTable *createComponentDataTable(
	uint32 numEntries,
	uint32 componentSize);
void freeComponentDataTable(ComponentDataTable **table);

void cdtInsert(ComponentDataTable *table, UUID entityID, void *componentData);
void cdtRemove(ComponentDataTable *table, UUID entityID);
void *cdtGet(ComponentDataTable *table, UUID entityID);

ComponentDataTableIterator cdtGetIterator(ComponentDataTable *table);
void cdtMoveIterator(ComponentDataTableIterator *itr);
uint32 cdtIteratorAtEnd(ComponentDataTableIterator itr);
UUID *cdtIteratorGetUUID(ComponentDataTableIterator itr) ;
void *cdtIteratorGetData(ComponentDataTableIterator itr);
