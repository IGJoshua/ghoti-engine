#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadCubemap(const char *name);
int32 uploadCubemapToGPU(Cubemap *cubemap);
Cubemap getCubemap(const char *name);
void freeCubemapData(Cubemap *cubemap);