#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

void exportSave(void *data, uint32 size, const Scene *scene, uint32 slot);
int32 loadSave(uint32 slot, Scene **scene);
bool getSaveSlotAvailability(uint32 slot);
int32 deleteSave(uint32 slot);