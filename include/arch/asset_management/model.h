#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

int32 loadModel(const char *name);
Model* getModel(const char *name);
void freeModel(const char *name);