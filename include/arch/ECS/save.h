#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

int32 exportSave(void *data, uint32 size, uint32 slot);
int32 loadSave(uint32 slot, void **data);
bool getSaveSlotAvailability(uint32 slot);
int32 deleteSave(uint32 slot);