#include "ECS/component.h"
#include "ECS/ecs_types.h"

#include "data/hash_map.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

ComponentDataTable *createComponentDataTable(
	uint32 numEntries,
	uint32 componentSize)
{
	ComponentDataTable *ret = malloc(
		sizeof(ComponentDataTable)
		+ numEntries
		* (componentSize + sizeof(UUID)));

	ASSERT(ret != 0);

	ret->componentSize = componentSize;
	ret->numEntries = numEntries;
	ret->idToIndex = createHashMap(
		sizeof(UUID),
		sizeof(uint32),
		CDT_ID_BUCKETS,
		(ComparisonOp)&strcmp);

	ASSERT(ret->idToIndex != 0);

	memset(ret->data, 0, numEntries * (componentSize + sizeof(UUID)));

	printf("Created a component data table with %d entries of %d bytes\n", numEntries, componentSize);

	return ret;
}

void freeComponentDataTable(ComponentDataTable **table)
{
	freeHashMap(&(*table)->idToIndex);
	free(*table);
	*table = 0;
}

void cdtInsert(ComponentDataTable *table, UUID entityID, void *componentData)
{
	uint32 i = 0;
	// If the entity is not in the table
	if (!hashMapGetKey(table->idToIndex, &entityID))
	{
		// Find an empty slot in the component data table
		UUID emptyID = {};
		for (; i < table->numEntries; ++i)
		{
			if (!strcmp((char *)(table->data
								 + i
								 * (table->componentSize + sizeof(UUID))),
						emptyID.string))
			{
				break;
			}
		}

		if (i >= table->numEntries)
		{
			return;
		}

		// Associate the UUID with the index in the map
		hashMapInsert(table->idToIndex, &entityID, &i);
	}
	// If the entity already exists in the table
	else
	{
		// Set i to the index
		i = *(uint32 *)hashMapGetKey(table->idToIndex, &entityID);
	}

	ASSERT(i >= 0);
	ASSERT(i < table->numEntries);
	ASSERT(i
		   * (table->componentSize + sizeof(UUID))
		   + sizeof(UUID)
		   + table->componentSize
		   <= table->numEntries * (table->componentSize + sizeof(UUID)));

	// Put the UUID into the table
	memcpy(table->data + i * (table->componentSize + sizeof(UUID)),
		   &entityID, sizeof(UUID));
	// Put the component data into the table
	memcpy(
		table->data
		+ i
		* (table->componentSize + sizeof(UUID))
		+ sizeof(UUID),
		componentData,
		table->componentSize);
}

void cdtRemove(
	ComponentDataTable *table,
	UUID entityID)
{
	// If the entity exists in the table
	uint32 *pIndex =
		hashMapGetKey(
			table->idToIndex,
			&entityID);
	if (pIndex)
	{
		memset(
			table->data
			+ *pIndex
			* (table->componentSize + sizeof(UUID)),
			0,
			sizeof(UUID));
		hashMapDeleteKey(table->idToIndex, &entityID);
	}
}

void *cdtGet(ComponentDataTable *table, UUID entityID)
{
	uint32 *index = hashMapGetKey(table->idToIndex, &entityID);

	if (index)
	{
		return
			(void *)table->data
			+ *index
			* (table->componentSize + sizeof(UUID))
			+ sizeof(UUID);
	}

	return 0;
}

ComponentDataTableIterator cdtGetIterator(ComponentDataTable *table)
{
	ComponentDataTableIterator itr = {};

	UUID emptyID = {};

	itr.index = 0;
	itr.table = table;

	if (!strcmp((const char *)&emptyID, (const char *)table->data))
	{
		cdtMoveIterator(&itr);
	}

	return itr;
}

void cdtMoveIterator(ComponentDataTableIterator *itr)
{
	if (itr->index >= itr->table->numEntries)
	{
		return;
	}

	UUID emptyID = {};

	while (!strcmp(
			   emptyID.string,
			   ((UUID *)(itr->table->data
						 + ++itr->index
						 * (itr->table->componentSize
							+ sizeof(UUID))))->string)
		   && itr->index < itr->table->numEntries)
		;
}

inline
uint32 cdtIteratorAtEnd(ComponentDataTableIterator itr)
{
	return itr.index >= itr.table->numEntries;
}

inline
UUID *cdtIteratorGetUUID(ComponentDataTableIterator itr)
{
	return (UUID *)(itr.table->data
					+ itr.index
					* (itr.table->componentSize
					   + sizeof(UUID)));
}

inline
void *cdtIteratorGetData(ComponentDataTableIterator itr)
{
	return (itr.table->data
			+ itr.index
			* (itr.table->componentSize
			   + sizeof(UUID)))
		+ sizeof(UUID);
}
