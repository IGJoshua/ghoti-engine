#include "ECS/component.h"
#include "ECS/ecs_types.h"

#include "core/log.h"

#include "data/hash_map.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

internal
inline
ComponentDataEntry *getEntry(ComponentDataTable *table, uint32 index)
{
	return (ComponentDataEntry *)
		(table->data
		 + index
		 * (table->componentSize + sizeof(ComponentDataEntry)));
}

ComponentDataTable *createComponentDataTable(
	uint32 numEntries,
	uint32 componentSize)
{
	ComponentDataTable *ret = malloc(
		sizeof(ComponentDataTable)
		+ numEntries
		* (componentSize + sizeof(ComponentDataEntry)));

	ASSERT(ret != 0);

	ret->componentSize = componentSize;
	ret->firstFree = 0;
	ret->numEntries = numEntries;
	ret->idToIndex = createHashMap(
		sizeof(UUID),
		sizeof(uint32),
		CDT_ID_BUCKETS,
		(ComparisonOp)&strcmp);

	ASSERT(ret->idToIndex != 0);

	for (uint32 i = 0; i < numEntries; ++i)
	{
		ComponentDataEntry *entry = getEntry(ret, i);

		entry->nextFree = i + 1;
	}

	LOG(
		"Created a component data table with %d entries of %d bytes\n",
		numEntries,
		componentSize);

	return ret;
}

void freeComponentDataTable(ComponentDataTable **table)
{
	freeHashMap(&(*table)->idToIndex);
	free(*table);
	*table = 0;
}

int32 cdtInsert(ComponentDataTable *table, UUID entityID, void *componentData)
{
	uint32 i = 0;
	// If the entity is not in the table
	if (!hashMapGetData(table->idToIndex, &entityID))
	{
		// Find an empty slot in the component data table
		if (table->firstFree >= table->numEntries)
		{
			return -1;
		}
		else
		{
			i = table->firstFree;
		}

		// Associate the UUID with the index in the map
		hashMapInsert(table->idToIndex, &entityID, &i);
	}
	// If the entity already exists in the table
	else
	{
		// Set i to the index
		i = *(uint32 *)hashMapGetData(table->idToIndex, &entityID);
	}

	ASSERT(i >= 0);
	ASSERT(i < table->numEntries);

	ComponentDataEntry *entry = getEntry(table, i);

	table->firstFree = entry->nextFree;

	// Put the UUID into the table
	memcpy(entry->entity.string, &entityID, sizeof(UUID));
	// Put the component data into the table
	memcpy(entry->data, componentData, table->componentSize);

	entry->nextFree = table->numEntries + 1;

	return 0;
}

void cdtRemove(
	ComponentDataTable *table,
	UUID entityID)
{
	// If the entity exists in the table
	uint32 *pIndex =
		hashMapGetData(
			table->idToIndex,
			&entityID);
	if (pIndex)
	{
		ComponentDataEntry *entry = getEntry(table, *pIndex);

		memset(
			entry->entity.string,
			0,
			sizeof(UUID));

		entry->nextFree = table->firstFree;
		table->firstFree = *pIndex;

		hashMapDeleteKey(table->idToIndex, &entityID);
	}
}

void *cdtGet(ComponentDataTable *table, UUID entityID)
{
	uint32 *index = hashMapGetData(table->idToIndex, &entityID);

	if (index)
	{
		return getEntry(table, *index)->data;
	}

	return 0;
}

ComponentDataTableIterator cdtGetIterator(ComponentDataTable *table)
{
	ComponentDataTableIterator itr = {};

	itr.index = 0;
	itr.table = table;

	if (getEntry(table, 0)->nextFree < table->numEntries + 1)
	{
		cdtMoveIterator(&itr);
	}

	return itr;
}

void cdtMoveIterator(ComponentDataTableIterator *itr)
{
	++itr->index;
	for (; itr->index < itr->table->numEntries; ++itr->index)
	{
		if (getEntry(itr->table, itr->index)->nextFree
			== itr->table->numEntries + 1)
		{
			break;
		}
	}
}

inline
uint32 cdtIteratorAtEnd(ComponentDataTableIterator itr)
{
	return itr.index >= itr.table->numEntries;
}

inline
UUID cdtIteratorGetUUID(ComponentDataTableIterator itr)
{
	return getEntry(itr.table, itr.index)->entity;
}

inline
void *cdtIteratorGetData(ComponentDataTableIterator itr)
{
	return getEntry(itr.table, itr.index)->data;
}
