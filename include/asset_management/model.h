#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

int32 loadModel(const char *name);
Model* getModel(const char *name);
uint32 getModelIndex(const char *name);
int32 freeModel(const char *name);
