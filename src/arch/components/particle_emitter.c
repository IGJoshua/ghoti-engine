#include "components/particle_emitter.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/scene.h"

HashMap particleEmitters = NULL;

internal real32 randomFloat(real32 min, real32 max);

void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform)
{
	if (particleList->numParticles + 1 > particleEmitter->maxNumParticles)
	{
		if (particleEmitter->stopAtCapacity)
		{
			particleEmitter->stopping = true;
		}

		return;
	}

	if (particleEmitter->stopping)
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

	return;
}

int32 removeParticle(
	ParticleEmitterComponent *particleEmitter,
	ParticleList *particleList,
	ListIterator *itr)
{
	listRemove(&particleList->particles, itr);
	particleList->numParticles--;

	if (particleEmitter->stopping && particleList->numParticles == 0)
	{
		stopParticleEmitter(particleEmitter, true);
		return -1;
	}

	return 0;
}

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
	bool loop)
{
	stopParticleEmitter(
		particleEmitter,
		strcmp(particleName, particleEmitter->currentParticle));

	if (strlen(particleName) > 0)
	{
		loadParticle(particleName, numSprites, spriteWidth, spriteHeight);
	}

	particleEmitter->stopAtCapacity = stopAtCapacity;
	strcpy(particleEmitter->currentParticle, particleName);
	particleEmitter->active = true;
	particleEmitter->initialSprite = initialSprite;
	particleEmitter->randomSprite = randomSprite;
	particleEmitter->animationFPS = animationFPS;
	particleEmitter->loop = loop;
}

void stopParticleEmitter(ParticleEmitterComponent *particleEmitter, bool reset)
{
	if (reset)
	{
		removeParticleEmitter(particleEmitter);
	}

	particleEmitter->active = false;
	particleEmitter->paused = false;
	particleEmitter->stopping = false;
	strcpy(particleEmitter->currentParticle, "");
	particleEmitter->particleCounter = 0.0;
}

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

real32 randomFloat(real32 min, real32 max)
{
    return min + (rand() / (RAND_MAX / (max - min)));
}