#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"
#include "asset_management/asset_manager_types.h"

#include <stdio.h>

int32 loadMask(Mask *mask, FILE *file);
int32 loadMaskTexture(const char *masksFolder, UUID textureName);
void freeMask(Mask *mask);