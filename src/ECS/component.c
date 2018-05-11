#include "ECS/component.h"

#include "ECS/ecs_types.h"

#include "data/hash_map.h"

#include <malloc.h>
#include <string.h>

ComponentDataTable *createComponentDataTable(uint32 numEntries, uint32 componentSize)
{
	ComponentDataTable *ret = malloc(sizeof(ComponentDataTable) + numEntries * componentSize);

	ret->componentSize = componentSize;
	ret->numEntries = numEntries;
	ret->idToIndex = createHashMap(sizeof(UUID), sizeof(uint32), numEntries, (ComparisonOp)&strcmp);

	memset(ret->data, 0, numEntries * componentSize);

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
			if (!strcmp((char *)(table->data + i * (table->componentSize + sizeof(UUID))), emptyID.string))
			{
				break;
			}
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

	// Put the UUID into the table
	memcpy(table->data + i * (table->componentSize + sizeof(UUID)),
		   &entityID, sizeof(UUID));
	// Put the component data into the table
	memcpy(table->data + i * (table->componentSize + sizeof(UUID)) + sizeof(UUID),
		   componentData, table->componentSize);
}

void cdtRemove(
	ComponentDataTable *table,
	UUID entityID)
{
	// If the entity exists in the table
	uint32 *pIndex =
		hashMapGetKey(
			table->idToIndex,
			&entityID
		);
	if (pIndex)
	{
		memset(
			table->data + *pIndex * (table->componentSize + sizeof(UUID)),
			0,
			sizeof(UUID)
		);
	}
}

void *cdtGet(ComponentDataTable *table, UUID entityID)
{
	uint32 *index = hashMapGetKey(table->idToIndex, &entityID);

	if (index)
	{
		return (void *)table->data + *index * (table->componentSize + sizeof(UUID)) + sizeof(UUID);
	}

	return 0;
}
