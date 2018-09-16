#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include <stdio.h>

int32 loadAnimations(
	uint32 *numAnimations,
	Animation **animations,
	Skeleton *skeleton,
	FILE *file);
Animation* getAnimation(Model *model, const char *name);
void freeAnimations(
	uint32 numAnimations,
	Animation *animations,
	Skeleton *skeleton);