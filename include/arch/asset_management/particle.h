#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadParticle(
	const char *name,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight);
int32 uploadParticleToGPU(Particle *particle);
Particle getParticle(const char *name);
void freeParticleData(Particle *particle);