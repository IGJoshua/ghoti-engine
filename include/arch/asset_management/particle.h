#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadParticle(
	const char *name,
	uint32 numSprites,
	uint32 rows,
	uint32 columns,
	bool textureFiltering);
int32 uploadParticleToGPU(Particle *particle);
Particle getParticle(const char *name);
void freeParticleData(Particle *particle);