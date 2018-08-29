#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

int32 loadFont(const char *name, uint32 size);
Font* getFont(const char *name, uint32 size);
void freeFont(Font *font);