#pragma once
#include "defines.h"

#include "ecs_types.h"

ComponentDataTable *createComponentDataTable(uint32 numEntries, uint32 componentSize);
void freeComponentDataTable(ComponentDataTable **table);

void cdtInsert(UUID entityID, void *componentData);
