#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

int32 loadParticle(
	const char *name,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight);
Particle* getParticle(const char *name);
void freeParticle(const char *name);