#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"
#include "asset_management/asset_manager_types.h"

#include <stdio.h>

void loadMask(Mask *mask, FILE *file);
int32 loadMaskTexture(
	const char *masksFolder,
	Model *model,
	char suffix,
	UUID *textureName);
void freeMask(Mask *mask);