#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/animation.h"

#include "components/component_types.h"
#include "components/animation.h"
#include "components/animator.h"
#include "components/transform.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/component.h"
#include "ECS/scene.h"

#include "math/math.h"

uint32 animationSystemRefCount = 0;

internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID nextAnimationComponentID = {};

#define SKELETONS_MAP_BUCKET_COUNT 7
#define SKELETONS_BUCKET_COUNT 2003
#define ANIMATIONS_BUCKET_COUNT 2003

extern HashMap skeletonsMap;
extern HashMap animationReferences;

internal HashMap skeletons;

internal void freeSceneSkeletons(Scene *scene);

internal kmVec3 getCurrentPosition(Bone *bone, real64 time);
internal kmQuaternion getCurrentRotation(Bone *bone, real64 time);
internal kmVec3 getCurrentScale(Bone *bone, real64 time);

internal int32 ptrCmp(void *a, void *b)
{
	return *(uint64*)a != *(uint64*)b;
}

internal void initAnimationSystem(Scene *scene)
{
	if (animationSystemRefCount == 0)
	{
		skeletonsMap = createHashMap(
			sizeof(Scene*),
			sizeof(HashMap),
			SKELETONS_MAP_BUCKET_COUNT,
			(ComparisonOp)&ptrCmp);

		animationReferences = createHashMap(
			sizeof(AnimatorComponent*),
			sizeof(AnimationReference),
			ANIMATIONS_BUCKET_COUNT,
			(ComparisonOp)&ptrCmp);
	}

	skeletons = createHashMap(
		sizeof(UUID),
		sizeof(HashMap),
		SKELETONS_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	hashMapInsert(skeletonsMap, &scene, &skeletons);

	ComponentDataTable *animationComponents =
		*(ComponentDataTable**)hashMapGetData(
				scene->componentTypes,
				&animationComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(animationComponents);
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		addSkeleton(
			scene,
			((AnimationComponent*)cdtIteratorGetData(itr))->skeleton);
	}

	animationSystemRefCount++;
}

internal void beginAnimationSystem(Scene *scene, real64 dt)
{
	skeletons = *(HashMap*)hashMapGetData(skeletonsMap, &scene);
}

internal void runAnimationSystem(Scene *scene, UUID entityID, real64 dt)
{
	AnimatorComponent *animator = sceneGetComponentFromEntity(
		scene,
		entityID,
		animatorComponentID);
	AnimationComponent *animationComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		animationComponentID);
	NextAnimationComponent *nextAnimation = sceneGetComponentFromEntity(
		scene,
		entityID,
		nextAnimationComponentID);

	AnimationReference *animationReference = hashMapGetData(
		animationReferences,
		&animator);

	if (!animationReference)
	{
		AnimationReference newAnimationReference;
		newAnimationReference.previousAnimation = NULL;
		newAnimationReference.currentAnimation = NULL;
		hashMapInsert(animationReferences, &animator, &newAnimationReference);

		animationReference = hashMapGetData(animationReferences, &animator);
	}

	if (!animationReference->currentAnimation)
	{
		ModelComponent *modelComponent = sceneGetComponentFromEntity(
			scene,
			entityID,
			modelComponentID);

		setCurrentAnimationReference(
			modelComponent,
			animator,
			animator->currentAnimation);

		if (!animationReference->currentAnimation)
		{
			if (nextAnimation)
			{
				playAnimation(
					modelComponent,
					animator,
					nextAnimation->name,
					nextAnimation->loopCount,
					nextAnimation->speed,
					nextAnimation->transitionDuration,
					false);

				strcpy(nextAnimation->name, "");
			}

			if (!animationReference->currentAnimation)
			{
				playAnimation(
					modelComponent,
					animator,
					animationComponent->idleAnimation,
					-1,
					animationComponent->speed,
					animationComponent->transitionDuration,
					false);
			}

			if (!animationReference->currentAnimation)
			{
				return;
			}
		}
	}

	if (animator->paused)
	{
		return;
	}

	HashMap *skeleton = hashMapGetData(
		skeletons,
		&animationComponent->skeleton);

	if (!skeleton)
	{
		addSkeleton(scene, animationComponent->skeleton);
		skeleton = hashMapGetData(skeletons, &animationComponent->skeleton);
	}

	for (uint32 i = 0; i < animationReference->currentAnimation->numBones; i++)
	{
		Bone *bone = &animationReference->currentAnimation->bones[i];

		JointTransform *jointTransform = hashMapGetData(*skeleton, &bone->name);
		if (jointTransform)
		{
			jointTransform->transform->position = getCurrentPosition(
				bone,
				animator->time);
			jointTransform->transform->rotation = getCurrentRotation(
				bone,
				animator->time);
			jointTransform->transform->scale = getCurrentScale(
				bone,
				animator->time);

			tMarkDirty(scene, jointTransform->uuid);
		}
	}

	real64 deltaTime = animator->speed * dt;

	if (animator->transitionTime < animator->transitionDuration)
	{
		if (animationReference->previousAnimation)
		{
			real32 t = animator->transitionTime / animator->transitionDuration;

			for (uint32 i = 0;
				 i < animationReference->previousAnimation->numBones;
				 i++)
			{
				Bone *bone = &animationReference->previousAnimation->bones[i];

				JointTransform *jointTransform = hashMapGetData(
					*skeleton,
					&bone->name);

				if (jointTransform)
				{
					kmVec3 position = getCurrentPosition(
						bone,
						animator->previousAnimationTime);
					kmQuaternion rotation = getCurrentRotation(
						bone,
						animator->previousAnimationTime);
					kmVec3 scale = getCurrentScale(
						bone,
						animator->previousAnimationTime);

					kmVec3Lerp(
						&jointTransform->transform->position,
						&position,
						&jointTransform->transform->position,
						t);
					quaternionSlerp(
						&jointTransform->transform->rotation,
						&rotation,
						&jointTransform->transform->rotation,
						t);
					kmVec3Lerp(
						&jointTransform->transform->scale,
						&scale,
						&jointTransform->transform->scale,
						t);

					tMarkDirty(scene, jointTransform->uuid);
				}
			}
		}

		animator->transitionTime += deltaTime < 0.0 ?
			deltaTime * -1.0 : deltaTime;
	}

	bool backwards = animator->speed < 0.0f;

	animator->time += deltaTime;

	bool stopped = false;
	if ((!backwards && animator->time > animator->duration) ||
		 (backwards && animator->time < 0.0))
	{
		if (animator->loopCount > -1)
		{
			if (--animator->loopCount == -1)
			{
				if (animator->time < 0.0)
				{
					animator->time = 0.0;
				}
				else
				{
					animator->time = animator->duration;
				}

				stopAnimation(animator);
				stopped = true;
			}
		}
	}

	if (!stopped)
	{
		while (animator->time < 0.0)
		{
			animator->time += animator->duration;
		}

		animator->time = fmod(animator->time, animator->duration);
	}
}

internal void shutdownAnimationSystem(Scene *scene)
{
	freeSceneSkeletons(scene);
	hashMapDelete(skeletonsMap, &scene);

	if (--animationSystemRefCount == 0)
	{
		freeHashMap(&skeletonsMap);
		freeHashMap(&animationReferences);
	}
}

System createAnimationSystem(void)
{
	System system = {};

	modelComponentID = idFromName("model");
	animationComponentID = idFromName("animation");
	animatorComponentID = idFromName("animator");
	nextAnimationComponentID = idFromName("next_animation");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &modelComponentID);
	listPushFront(&system.componentTypes, &animationComponentID);
	listPushFront(&system.componentTypes, &animatorComponentID);

	system.init = &initAnimationSystem;
	system.begin = &beginAnimationSystem;
	system.run = &runAnimationSystem;
	system.shutdown = &shutdownAnimationSystem;

	return system;
}

void freeSceneSkeletons(Scene *scene)
{
	HashMap *skeletons = hashMapGetData(skeletonsMap, &scene);
	for (HashMapIterator itr = hashMapGetIterator(*skeletons);
		!hashMapIteratorAtEnd(itr);
		hashMapMoveIterator(&itr))
	{
		freeHashMap(hashMapIteratorGetValue(itr));
	}

	freeHashMap(skeletons);
}

kmVec3 getCurrentPosition(Bone *bone, real64 time)
{
	if (bone->numPositionKeyFrames == 1)
	{
		return bone->positionKeyFrames[0].value;
	}

	uint32 a = 0, b = 0;
	for (uint32 i = 0; i < bone->numPositionKeyFrames - 1; i++)
	{
		if (time <= bone->positionKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	Vec3KeyFrame *keyFrameA = &bone->positionKeyFrames[a];
	Vec3KeyFrame *keyFrameB = &bone->positionKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 position;
	kmVec3Lerp(&position, &keyFrameA->value, &keyFrameB->value, t);

	return position;
}

kmQuaternion getCurrentRotation(Bone *bone, real64 time)
{
	if (bone->numRotationKeyFrames == 1)
	{
		return bone->rotationKeyFrames[0].value;
	}

	uint32 a = 0, b = 0;
	for (uint32 i = 0; i < bone->numRotationKeyFrames - 1; i++)
	{
		if (time <= bone->rotationKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	QuaternionKeyFrame *keyFrameA = &bone->rotationKeyFrames[a];
	QuaternionKeyFrame *keyFrameB = &bone->rotationKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmQuaternion rotation;
	quaternionSlerp(&rotation, &keyFrameA->value, &keyFrameB->value, t);

	return rotation;
}

kmVec3 getCurrentScale(Bone *bone, real64 time)
{
	if (bone->numScaleKeyFrames == 1)
	{
		return bone->scaleKeyFrames[0].value;
	}

	uint32 a = 0, b = 0;
	for (uint32 i = 0; i < bone->numScaleKeyFrames - 1; i++)
	{
		if (time <= bone->scaleKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	Vec3KeyFrame *keyFrameA = &bone->scaleKeyFrames[a];
	Vec3KeyFrame *keyFrameB = &bone->scaleKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 scale;
	kmVec3Lerp(&scale, &keyFrameA->value, &keyFrameB->value, t);

	return scale;
}