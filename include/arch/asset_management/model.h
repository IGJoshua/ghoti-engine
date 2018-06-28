#pragma once
#include "defines.h"

#include <kazmath/mat4.h>

#include "asset_management/asset_manager_types.h"

int32 loadModel(const char *name);
void increaseModelsCapacity();
Model* getModel(const char *name);
uint32 getModelIndex(const char *name);
int32 freeModel(const char *name);
