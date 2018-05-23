#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "renderer/renderer_types.h"

#include <kazmath/mat4.h>

int32 loadModel(const char *name);
Model* getModel(const char *name);
uint32 getModelIndex(const char *name);
int32 freeModel(const char *name);
int32 renderModel(const char *name, kmMat4 *world, kmMat4 *view, kmMat4 *projection);