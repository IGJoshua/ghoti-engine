#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadFont(const char *name, real32 size, bool autoScaling);
int32 uploadFontToGPU(Font *font);
Font getFont(const char *name, real32 size, bool autoScaling);
void freeFontData(Font *font);