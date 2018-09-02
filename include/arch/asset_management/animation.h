#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <stdio.h>

int32 loadAnimations(
	uint32 *numAnimations,
	Animation **animations,
	Skeleton *skeleton,
	FILE *file);
void freeAnimations(
	uint32 numAnimations,
	Animation *animations,
	Skeleton *skeleton);