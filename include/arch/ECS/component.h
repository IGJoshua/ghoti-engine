#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#define CDT_ID_BUCKETS 10007

ComponentDataTable *createComponentDataTable(
	UUID componentID,
	uint32 numEntries,
	uint32 componentSize);
void freeComponentDataTable(ComponentDataTable **table);

int32 cdtInsert(ComponentDataTable *table, UUID entityID, void *componentData);
void cdtRemove(ComponentDataTable *table, UUID entityID);
void *cdtGet(ComponentDataTable *table, UUID entityID);
UUID cdtGetIndexUUID(ComponentDataTable *table, uint32 index);
void *cdtGetIndexData(ComponentDataTable *table, uint32 index);
uint32 cdtGetIndexNF(ComponentDataTable *table, uint32 index);

ComponentDataTableIterator cdtGetIterator(ComponentDataTable *table);
void cdtMoveIterator(ComponentDataTableIterator *itr);
uint32 cdtIteratorAtEnd(ComponentDataTableIterator itr);
UUID cdtIteratorGetUUID(ComponentDataTableIterator itr) ;
void *cdtIteratorGetData(ComponentDataTableIterator itr);
