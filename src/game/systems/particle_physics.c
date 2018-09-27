#include "defines.h"

#include "components/component_types.h"
#include "components/particle_emitter.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include <malloc.h>

internal uint32 particlePhysicsSystemRefCount = 0;

internal UUID transformComponentID = {};
internal UUID particleEmitterComponentID = {};
internal UUID rigidBodyComponentID = {};

#define PARTICLE_EMITTERS_BUCKET_COUNT 127

extern HashMap particleEmitters;

internal int32 ptrcmp(void *a, void *b)
{
	return *(uint64*)a != *(uint64*)b;
}

internal void initParticlePhysicsSystem(Scene *scene)
{
	if (particlePhysicsSystemRefCount == 0)
	{
		particleEmitters = createHashMap(
			sizeof(ParticleEmitterComponent*),
			sizeof(ParticleList),
			PARTICLE_EMITTERS_BUCKET_COUNT,
			(ComparisonOp)&ptrcmp);
	}

	particlePhysicsSystemRefCount++;
}

internal void runParticlePhysicsSystem(Scene *scene, UUID entityID, real64 dt)
{
	ParticleEmitterComponent *particleEmitter = sceneGetComponentFromEntity(
		scene,
		entityID,
		particleEmitterComponentID);

	if (!particleEmitter->active)
	{
		return;
	}

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	ParticleList *particleList = hashMapGetData(
		particleEmitters,
		&particleEmitter);

	if (!particleList)
	{
		ParticleList newParticleList = {};
		newParticleList.particles = createList(sizeof(ParticleObject));

		hashMapInsert(particleEmitters, &particleEmitter, &newParticleList);
		particleList = hashMapGetData(particleEmitters, &particleEmitter);

		addParticle(particleList, particleEmitter, transform);
	}

	if (!particleEmitter->paused)
	{
		particleEmitter->particleCounter += particleEmitter->spawnRate * dt;
		while (particleEmitter->particleCounter >= 1.0)
		{
			addParticle(particleList, particleEmitter, transform);
			particleEmitter->particleCounter -= 1.0;
		}

		for (ListIterator listItr = listGetIterator(&particleList->particles);
			!listIteratorAtEnd(listItr);)
		{
			ParticleObject *particle = LIST_ITERATOR_GET_ELEMENT(
				ParticleObject,
				listItr);

			if (particle->lifetime <= 0.0)
			{
				listMoveIterator(&listItr);
				continue;
			}

			particle->lifetime -= dt;
			if (particle->lifetime <= 0.0)
			{
				if (removeParticle(
					particleEmitter,
					particleList,
					&listItr) == -1)
				{
					return;
				}
			}
			else
			{
				if (particle->lifetime <= particleEmitter->fadeTime)
				{
					particle->fadeTimer += dt;
					particle->color.w = kmLerp(
						particleEmitter->alpha,
						0.0f,
						particle->fadeTimer / particleEmitter->fadeTime);
				}

				listMoveIterator(&listItr);
			}
		}
	}

	for (ListIterator listItr = listGetIterator(&particleList->particles);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		ParticleObject *particle = LIST_ITERATOR_GET_ELEMENT(
			ParticleObject,
			listItr);
		kmVec3Assign(&particle->previousPosition, &particle->position);
	}

	if (particleEmitter->paused)
	{
		return;
	}

	for (ListIterator listItr = listGetIterator(&particleList->particles);
		 !listIteratorAtEnd(listItr);
		 listMoveIterator(&listItr))
	{
		ParticleObject *particle = LIST_ITERATOR_GET_ELEMENT(
			ParticleObject,
			listItr);

		kmVec3 deltaVelocity;
		kmVec3Scale(&deltaVelocity, &particleEmitter->acceleration, dt);
		kmVec3Add(&particle->velocity, &particle->velocity, &deltaVelocity);

		kmVec3 displacement;
		kmVec3Scale(&displacement, &particle->velocity, dt);
		kmVec3Add(&particle->position, &particle->position, &displacement);
	}
}

internal void shutdownParticlePhysicsSystem(Scene *scene)
{
	if (--particlePhysicsSystemRefCount == 0)
	{
		for (HashMapIterator itr = hashMapGetIterator(particleEmitters);
			 !hashMapIteratorAtEnd(itr);
			 hashMapMoveIterator(&itr))
		{
			ParticleList *particleList = hashMapIteratorGetValue(itr);
			listClear(&particleList->particles);
		}

		freeHashMap(&particleEmitters);
	}
}

System createParticlePhysicsSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	particleEmitterComponentID = idFromName("particle_emitter");
	rigidBodyComponentID = idFromName("rigid_body");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &particleEmitterComponentID);

	system.init = &initParticlePhysicsSystem;
	system.run = &runParticlePhysicsSystem;
	system.shutdown = &shutdownParticlePhysicsSystem;

	return system;
}