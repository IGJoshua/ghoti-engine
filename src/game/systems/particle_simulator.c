#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/particle.h"

#include "components/component_types.h"
#include "components/particle_emitter.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "math/math.h"

#include <malloc.h>

internal uint32 particleSimulatorSystemRefCount = 0;

internal UUID transformComponentID = {};
internal UUID particleEmitterComponentID = {};
internal UUID particleComponentID = {};

#define PARTICLE_EMITTERS_BUCKET_COUNT 127

extern HashMap particleEmitters;

internal ParticleList* getParticleList(
	UUID entity,
	ParticleEmitterComponent *particleEmitter);

internal int32 ptrcmp(void *a, void *b)
{
	return *(uint64*)a != *(uint64*)b;
}

internal void initParticleSimulatorSystem(Scene *scene)
{
	if (particleSimulatorSystemRefCount == 0)
	{
		particleEmitters = createHashMap(
			sizeof(ParticleEmitterReference),
			sizeof(ParticleList),
			PARTICLE_EMITTERS_BUCKET_COUNT,
			(ComparisonOp)&ptrcmp);
	}

	ComponentDataTable **particleComponents =
		(ComponentDataTable**)hashMapGetData(
			scene->componentTypes,
			&particleComponentID);

	if (particleComponents && *particleComponents)
	{
		for (ComponentDataTableIterator itr =
				 cdtGetIterator(*particleComponents);
			 !cdtIteratorAtEnd(itr);)
		{
			ParticleComponent *particleComponent = cdtIteratorGetData(itr);

			ParticleEmitterComponent *particleEmitter =
				sceneGetComponentFromEntity(
					scene,
					particleComponent->particleEmitter,
					particleEmitterComponentID);

			if (particleEmitter)
			{
				ParticleObject particle;
				memcpy(
					&particle,
					&particleComponent->lifetime,
					sizeof(ParticleObject));

				ParticleList *particleList = getParticleList(
					particleComponent->particleEmitter,
					particleEmitter);

				listPushBack(&particleList->particles, &particle);
				particleList->numParticles++;
			}

			UUID entity = cdtIteratorGetUUID(itr);
			cdtMoveIterator(&itr);
			sceneRemoveEntity(scene, entity);
		}

		sceneRegisterEntity(scene, particleComponentID);

		ParticleComponent particlePrototype = {};
		sceneAddComponentToEntity(
			scene,
			particleComponentID,
			particleComponentID,
			&particlePrototype);
	}

	particleSimulatorSystemRefCount++;
}

internal void runParticleSimulatorSystem(Scene *scene, UUID entityID, real64 dt)
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

	ParticleList *particleList = getParticleList(entityID, particleEmitter);

	if (!particleEmitter->paused)
	{
		particleEmitter->particleCounter +=
			particleEmitter->currentSpawnRate * dt;

		if (particleEmitter->particleCounter >= 1.0)
		{
			particleEmitter->currentSpawnRate = randomRealNumber(
				particleEmitter->spawnRate[0],
				particleEmitter->spawnRate[1]);
		}

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
					entityID,
					particleEmitter,
					particleList,
					&listItr) == -1)
				{
					return;
				}
			}
			else
			{
				if (particle->lifetime <= particle->fadeTime[1])
				{
					particle->fadeTimer[1] += dt;
					particle->color.w = kmLerp(
						particle->alpha,
						0.0f,
						particle->fadeTimer[1] / particle->fadeTime[1]);
				}
				else if (particle->fadeTimer[0] < particle->fadeTime[0])
				{
					particle->fadeTimer[0] += dt;
					particle->color.w = kmLerp(
						0.0f,
						particle->alpha,
						particle->fadeTimer[0] / particle->fadeTime[0]);
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

		Particle particleTexture = getParticle(
			particleEmitter->currentParticle);
		if (strlen(particleTexture.name.string) > 0)
		{
			uint32 lastSprite = particleTexture.numSprites - 1;

			if (particle->sprite == -1)
			{
				if (particleEmitter->randomSprite)
				{
					particle->sprite = randomInteger(0, lastSprite);
				}
				else
				{
					particle->sprite = particleEmitter->initialSprite;
					if (particle->sprite == -1)
					{
						switch (particleEmitter->animationMode)
						{
							case PARTICLE_ANIMATION_FORWARD:
							case PARTICLE_ANIMATION_LOOP_FORWARD:
							case PARTICLE_ANIMATION_BOUNCING_FORWARD:
								particle->sprite = 0;
								break;
							case PARTICLE_ANIMATION_BACKWARD:
							case PARTICLE_ANIMATION_LOOP_BACKWARD:
							case PARTICLE_ANIMATION_BOUNCING_BACKWARD:
								particle->sprite = lastSprite;
								break;
							default:
								break;
						}
					}
				}

				switch (particleEmitter->animationMode)
				{
					case PARTICLE_ANIMATION_BOUNCING_FORWARD:
						particle->animationDirection = 1;
						break;
					case PARTICLE_ANIMATION_BOUNCING_BACKWARD:
						particle->animationDirection = -1;
						break;
					default:
						break;
				}
			}

			switch (particleEmitter->animationMode)
			{
				case PARTICLE_ANIMATION_FORWARD:
				case PARTICLE_ANIMATION_LOOP_FORWARD:
					particle->animationDirection = 1;
					break;
				case PARTICLE_ANIMATION_BACKWARD:
				case PARTICLE_ANIMATION_LOOP_BACKWARD:
					particle->animationDirection = -1;
					break;
				default:
					break;
			}

			particle->animationTime += particleEmitter->animationFPS * dt;
			while (particle->animationTime >= 1.0)
			{
				int32 nextSprite =
					particle->sprite + particle->animationDirection;

				switch (particleEmitter->animationMode)
				{
					case PARTICLE_ANIMATION_FORWARD:
						if (particle->sprite != particleEmitter->finalSprite)
						{
							particle->sprite = nextSprite %
								particleTexture.numSprites;
						}

						break;
					case PARTICLE_ANIMATION_BACKWARD:
						if (particle->sprite != particleEmitter->finalSprite)
						{
							if (nextSprite == -1)
							{
								particle->sprite = lastSprite;
							}
							else
							{
								particle->sprite = nextSprite;
							}
						}

						break;
					case PARTICLE_ANIMATION_LOOP_FORWARD:
						particle->sprite = nextSprite %
							particleTexture.numSprites;
						break;
					case PARTICLE_ANIMATION_LOOP_BACKWARD:
						if (nextSprite == -1)
						{
							particle->sprite = lastSprite;
						}
						else
						{
							particle->sprite = nextSprite;
						}

						break;
					case PARTICLE_ANIMATION_BOUNCING_FORWARD:
					case PARTICLE_ANIMATION_BOUNCING_BACKWARD:
						if (particle->animationDirection == 1)
						{
							if (nextSprite == particleTexture.numSprites)
							{
								particle->animationDirection = -1;
								nextSprite = particle->sprite - 1;
							}
						}
						else
						{
							if (nextSprite == -1)
							{
								particle->animationDirection = 1;
								nextSprite = particle->sprite + 1;
							}
						}

						particle->sprite = nextSprite;

						break;
					default:
						break;
				}

				particle->animationTime -= 1.0;
			}

			kmVec2Assign(
				&particle->uv,
				&particleTexture.spriteUVs[particle->sprite]);
		}
	}
}

internal void shutdownParticleSimulatorSystem(Scene *scene)
{
	ComponentDataTable **particleEmitterComponents =
		(ComponentDataTable**)hashMapGetData(
			scene->componentTypes,
			&particleEmitterComponentID);

	for (HashMapIterator itr = hashMapGetIterator(particleEmitters);
		 !hashMapIteratorAtEnd(itr);)
	{
		ParticleEmitterReference *particleEmitterReference =
			hashMapIteratorGetKey(itr);

		bool inScene = false;
		for (ComponentDataTableIterator itr =
				 cdtGetIterator(*particleEmitterComponents);
			 !cdtIteratorAtEnd(itr);)
		{
			ParticleEmitterComponent *particleEmitterComponent =
				cdtIteratorGetData(itr);
			if (particleEmitterComponent ==
				particleEmitterReference->particleEmitter)
			{
				inScene = true;
				break;
			}
		}

		if (!inScene)
		{
			hashMapMoveIterator(&itr);
			continue;
		}

		ParticleList *particleList = hashMapIteratorGetValue(itr);

		for (ListIterator listItr =
				 listGetIterator(&particleList->particles);
			 !listIteratorAtEnd(listItr);
			 listMoveIterator(&listItr))
		{
			ParticleObject *particleObject = LIST_ITERATOR_GET_ELEMENT(
				ParticleObject,
				listItr);

			ParticleComponent particleComponent;
			particleComponent.particleEmitter =
				particleEmitterReference->entity;
			memcpy(
				&particleComponent.lifetime,
				particleObject,
				sizeof(ParticleObject));

			UUID particleEntity = sceneCreateEntity(scene);
			sceneAddComponentToEntity(
				scene,
				particleEntity,
				particleComponentID,
				&particleComponent);
		}

		listClear(&particleList->particles);

		ParticleEmitterReference reference = *particleEmitterReference;
		hashMapMoveIterator(&itr);
		hashMapDelete(particleEmitters, &reference);
	}

	if (--particleSimulatorSystemRefCount == 0)
	{
		freeHashMap(&particleEmitters);
	}
}

System createParticleSimulatorSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	particleEmitterComponentID = idFromName("particle_emitter");
	particleComponentID = idFromName("particle");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &particleEmitterComponentID);

	system.init = &initParticleSimulatorSystem;
	system.run = &runParticleSimulatorSystem;
	system.shutdown = &shutdownParticleSimulatorSystem;

	return system;
}


ParticleList* getParticleList(
	UUID entity,
	ParticleEmitterComponent *particleEmitter)
{
	ParticleEmitterReference particleEmitterReference;
	particleEmitterReference.entity = entity;
	particleEmitterReference.particleEmitter = particleEmitter;

	ParticleList *particleList = hashMapGetData(
		particleEmitters,
		&particleEmitterReference);

	if (!particleList)
	{
		ParticleList newParticleList = {};
		newParticleList.particles = createList(sizeof(ParticleObject));

		hashMapInsert(
			particleEmitters,
			&particleEmitterReference,
			&newParticleList);
		particleList = hashMapGetData(
			particleEmitters,
			&particleEmitterReference);

		particleEmitter->particleCounter = 1.0;
	}

	return particleList;
}