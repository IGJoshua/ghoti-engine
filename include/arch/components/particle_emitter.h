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

void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform);
int32 removeParticle(
	ParticleEmitterComponent *particleEmitter,
	ParticleList *particleList,
	ListIterator *itr);

void emitParticles(
	ParticleEmitterComponent *particleEmitter,
	bool stopAtCapacity,
	const char *particleName,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight,
	int32 initialSprite,
	bool randomSprite,
	real32 animationFPS,
	bool loop);
void stopParticleEmitter(ParticleEmitterComponent *particleEmitter, bool reset);

void removeParticleEmitter(ParticleEmitterComponent *particleEmitter);