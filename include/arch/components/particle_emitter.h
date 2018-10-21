#pragma once
#include "defines.h"

#include "components/component_types.h"

typedef struct particle_object_t
{
	real64 lifetime;
	real64 fadeTimer[2];
	real64 fadeTime[2];
	int32 sprite;
	real64 animationTime;
	int8 animationDirection;
	kmVec3 position;
	kmVec3 previousPosition;
	kmVec3 velocity;
	kmVec2 size;
	kmVec2 uv;
	kmVec4 color;
	real32 alpha;
} ParticleObject;

typedef struct emitted_particles_t
{
	uint32 numParticles;
	List particles;
} ParticleList;

typedef struct particle_emitter_reference_t
{
	UUID entity;
	ParticleEmitterComponent *particleEmitter;
} ParticleEmitterReference;

void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform);
int32 removeParticle(
	UUID entity,
	ParticleEmitterComponent *particleEmitter,
	ParticleList *particleList,
	ListIterator *itr);

void emitParticles(
	UUID entity,
	ParticleEmitterComponent *particleEmitter,
	bool stopAtCapacity,
	const char *particleName,
	uint32 numSprites,
	uint32 rows,
	uint32 columns,
	bool textureFiltering,
	int32 initialSprite,
	bool randomSprite,
	real32 animationFPS,
	ParticleAnimation animationMode,
	uint32 finalSprite);
void stopParticleEmitter(
	UUID entity,
	ParticleEmitterComponent *particleEmitter,
	bool reset);

void removeParticleEmitter(
	UUID entity,
	ParticleEmitterComponent *particleEmitter);