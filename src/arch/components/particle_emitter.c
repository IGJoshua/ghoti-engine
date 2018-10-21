#include "components/particle_emitter.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/scene.h"

#include "math/math.h"

HashMap particleEmitters = NULL;

void addParticle(
	ParticleList *particleList,
	ParticleEmitterComponent *particleEmitter,
	TransformComponent *transform)
{
	if (particleList->numParticles == particleEmitter->maxNumParticles)
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

	particle.lifetime = randomRealNumber(
		particleEmitter->lifetime[0],
		particleEmitter->lifetime[1]);

	if (particleEmitter->fadeTime[0] + particleEmitter->fadeTime[1] > 1.0f)
	{
		real32 sum = particleEmitter->fadeTime[0] +
					 particleEmitter->fadeTime[1];
		particleEmitter->fadeTime[0] /= sum;
		particleEmitter->fadeTime[1] /= sum;
	}

	particle.fadeTime[0] = particleEmitter->fadeTime[0] * particle.lifetime;
	particle.fadeTime[1] = particleEmitter->fadeTime[1] * particle.lifetime;

	kmVec3Assign(&particle.position, &transform->globalPosition);
	kmVec3Assign(&particle.velocity, &particleEmitter->initialVelocity);

	particle.sprite = -1;

	kmVec3 randomVelocity;
	kmVec3Fill(
		&randomVelocity,
		randomRealNumber(
			particleEmitter->minRandomVelocity.x,
			particleEmitter->maxRandomVelocity.x),
		randomRealNumber(
			particleEmitter->minRandomVelocity.y,
			particleEmitter->maxRandomVelocity.y),
		randomRealNumber(
			particleEmitter->minRandomVelocity.z,
			particleEmitter->maxRandomVelocity.z));

	kmVec3Add(&particle.velocity, &particle.velocity, &randomVelocity);

	if (particleEmitter->preserveAspectRatio)
	{
		real32 sizeRatio = randomRealNumber(0.0f, 1.0f);
		kmVec2Lerp(
			&particle.size,
			&particleEmitter->minSize,
			&particleEmitter->maxSize,
			sizeRatio);
	}
	else
	{
		kmVec2Fill(
			&particle.size,
			randomRealNumber(
				particleEmitter->minSize.x,
				particleEmitter->maxSize.x),
			randomRealNumber(
				particleEmitter->minSize.y,
				particleEmitter->maxSize.y));
	}

	kmVec4Fill(
		&particle.color,
		particleEmitter->color.x,
		particleEmitter->color.y,
		particleEmitter->color.z,
		particleEmitter->color.w);

	kmVec4 randomColor;
	kmVec4Fill(
		&randomColor,
		randomRealNumber(
			particleEmitter->minRandomColor.x,
			particleEmitter->maxRandomColor.x),
		randomRealNumber(
			particleEmitter->minRandomColor.y,
			particleEmitter->maxRandomColor.y),
		randomRealNumber(
			particleEmitter->minRandomColor.z,
			particleEmitter->maxRandomColor.z),
		randomRealNumber(
			particleEmitter->minRandomColor.w,
			particleEmitter->maxRandomColor.w));

	kmVec4Mul(&particle.color, &particle.color, &randomColor);
	particle.alpha = particle.color.w;

	listPushBack(&particleList->particles, &particle);
	particleList->numParticles++;

	return;
}

int32 removeParticle(
	UUID entity,
	ParticleEmitterComponent *particleEmitter,
	ParticleList *particleList,
	ListIterator *itr)
{
	listRemove(&particleList->particles, itr);
	particleList->numParticles--;

	if (particleEmitter->stopping && particleList->numParticles == 0)
	{
		stopParticleEmitter(entity, particleEmitter, true);
		return -1;
	}

	return 0;
}

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
	uint32 finalSprite)
{
	stopParticleEmitter(
		entity,
		particleEmitter,
		strcmp(particleName, particleEmitter->currentParticle));

	if (strlen(particleName) > 0)
	{
		loadParticle(particleName, numSprites, rows, columns, textureFiltering);
	}

	particleEmitter->stopAtCapacity = stopAtCapacity;
	strcpy(particleEmitter->currentParticle, particleName);
	particleEmitter->active = true;
	particleEmitter->initialSprite = initialSprite;
	particleEmitter->randomSprite = randomSprite;
	particleEmitter->animationFPS = animationFPS;
	particleEmitter->animationMode = animationMode;
	particleEmitter->finalSprite = finalSprite;
}

void stopParticleEmitter(
	UUID entity,
	ParticleEmitterComponent *particleEmitter,
	bool reset)
{
	if (reset)
	{
		removeParticleEmitter(entity, particleEmitter);
	}

	particleEmitter->active = false;
	particleEmitter->paused = false;
	particleEmitter->stopping = false;
	strcpy(particleEmitter->currentParticle, "");
	particleEmitter->particleCounter = 0.0;
}

void removeParticleEmitter(
	UUID entity,
	ParticleEmitterComponent *particleEmitter)
{
	if (particleEmitters)
	{
		ParticleEmitterReference particleEmitterReference;
		particleEmitterReference.entity = entity;
		particleEmitterReference.particleEmitter = particleEmitter;

		ParticleList *particleList = hashMapGetData(
			particleEmitters,
			&particleEmitterReference);

		if (particleList)
		{
			listClear(&particleList->particles);
			hashMapDelete(particleEmitters, &particleEmitterReference);
		}
	}
}