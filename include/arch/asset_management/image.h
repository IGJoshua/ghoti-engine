#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

int32 loadImage(const char *name);
Image* getImage(const char *name);
void freeImage(const char *name);