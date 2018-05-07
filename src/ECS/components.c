#include "ECS/components.h"

#include "ECS/ecs_types.h"

#include "data/hash_map.h"

#include <malloc.h>
#include <string.h>

ComponentDataTable *createComponentDataTable(uint32 numEntries, uint32 componentSize)
{
	ComponentDataTable *ret = malloc(sizeof(ComponentDataTable) + numEntries * componentSize);

	ret->componentSize = componentSize;
	ret->numEntries = numEntries;
	ret->idToIndex = createHashMap(64, sizeof(uint32), numEntries, (ComparisonOp)&strcmp);

	memset(ret->data, 0, numEntries * componentSize);

	return ret;
}

void freeComponentDataTable(ComponentDataTable **table)
{
	deleteHashMap(&(*table)->idToIndex);
	free(*table);
	*table = 0;
}
