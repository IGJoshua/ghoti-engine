#pragma once
#include "defines.h"

#include "components/component_types.h"

typedef struct particle_object_t
{
	real64 lifetime;
	real64 fadeTimer;
	kmVec3 position;
	kmVec3 previousPosition;
	kmVec3 velocity;
	kmVec4 color;
} ParticleObject;

typedef struct emitted_particles_t
{
	uint32 numParticles;
	List particles;
} ParticleList;

void removeParticleEmitter(ParticleEmitterComponent *particleEmitter);
void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform);
void removeParticle(ParticleList *particleList, ListIterator *itr);