#include "components/particle_emitter.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/scene.h"

extern HashMap particleEmitters;

internal real32 randomFloat(real32 min, real32 max);

void removeParticleEmitter(ParticleEmitterComponent *particleEmitter)
{
	if (particleEmitters)
	{
		ParticleList *particleList = hashMapGetData(
			particleEmitters,
			&particleEmitter);

		if (particleList)
		{
			listClear(&particleList->particles);
			hashMapDelete(particleEmitters, &particleEmitter);
		}
	}
}

void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform)
{
	if (particleList->numParticles + 1 > particleEmitter->maxNumParticles)
	{
		return;
	}

	ParticleObject particle = {};

	particle.lifetime = particleEmitter->lifetime;
	kmVec3Assign(&particle.position, &transform->globalPosition);
	kmVec3Assign(&particle.velocity, &particleEmitter->initialVelocity);

	kmVec3 randomVelocity;
	kmVec3Fill(
		&randomVelocity,
		randomFloat(
			particleEmitter->minRandomVelocity.x,
			particleEmitter->maxRandomVelocity.x),
		randomFloat(
			particleEmitter->minRandomVelocity.y,
			particleEmitter->maxRandomVelocity.y),
		randomFloat(
			particleEmitter->minRandomVelocity.z,
			particleEmitter->maxRandomVelocity.z));

	kmVec3Add(&particle.velocity, &particle.velocity, &randomVelocity);

	kmVec4Fill(
		&particle.color,
		particleEmitter->color.x,
		particleEmitter->color.y,
		particleEmitter->color.z,
		particleEmitter->alpha);

	kmVec4 randomColor;
	kmVec4Fill(
		&randomColor,
		randomFloat(
			particleEmitter->minRandomColor.x,
			particleEmitter->maxRandomColor.x),
		randomFloat(
			particleEmitter->minRandomColor.y,
			particleEmitter->maxRandomColor.y),
		randomFloat(
			particleEmitter->minRandomColor.z,
			particleEmitter->maxRandomColor.z),
		1.0f);

	kmVec4Mul(&particle.color, &particle.color, &randomColor);

	listPushBack(&particleList->particles, &particle);
	particleList->numParticles++;
}

void removeParticle(ParticleList *particleList, ListIterator *itr)
{
	listRemove(&particleList->particles, itr);
	particleList->numParticles--;
}

real32 randomFloat(real32 min, real32 max)
{
    return min + (rand() / (RAND_MAX / (max - min)));
}