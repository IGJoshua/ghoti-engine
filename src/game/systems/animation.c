#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"

#include "components/component_types.h"
#include "components/animation.h"
#include "components/animator.h"
#include "components/transform.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "math/math.h"

internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID nextAnimationComponentID = {};

internal kmVec3 getCurrentPosition(Bone *bone, real64 time, bool backwards);
internal kmQuaternion getCurrentRotation(
	Bone *bone,
	real64 time,
	bool backwards);
internal kmVec3 getCurrentScale(Bone *bone, real64 time, bool backwards);

internal void runAnimationSystem(Scene *scene, UUID entityID, real64 dt)
{
	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);
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

	Model *model = getModel(modelComponent->name);
	if (!model || model->numAnimations == 0)
	{
		stopAnimation(animator);
		return;
	}

	Animation *animation = NULL;
	if (strlen(animator->currentAnimation) > 0)
	{
		for (uint32 i = 0; i < model->numAnimations; i++)
		{
			if (!strcmp(
				model->animations[i].name.string,
				animator->currentAnimation))
			{
				animation = &model->animations[i];
				break;
			}
		}
	}

	if (!animation)
	{
		if (strlen(nextAnimation->name) > 0)
		{
			for (uint32 i = 0; i < model->numAnimations; i++)
			{
				if (!strcmp(
					model->animations[i].name.string,
					nextAnimation->name))
				{
					animation = &model->animations[i];
					break;
				}
			}
		}

		if (animation)
		{
			playAnimation(
				modelComponent,
				animator,
				nextAnimation->name,
				nextAnimation->loopCount,
				nextAnimation->speed,
				nextAnimation->backwards);

			strcpy(nextAnimation->name, "");
		}
		else
		{
			playAnimation(
				modelComponent,
				animator,
				animationComponent->idleAnimation,
				-1,
				animationComponent->speed,
				animationComponent->backwards);

			for (uint32 i = 0; i < model->numAnimations; i++)
			{
				if (!strcmp(
					model->animations[i].name.string,
					animator->currentAnimation))
				{
					animation = &model->animations[i];
					break;
				}
			}
		}

		if (!animation)
		{
			stopAnimation(animator);
			return;
		}
	}

	if (animator->paused)
	{
		return;
	}

	for (uint32 i = 0; i < animation->numBones; i++) {
		Bone *bone = &animation->bones[i];

		UUID joint;
		TransformComponent *jointTransform = getJointTransform(
			scene,
			animationComponent->skeleton,
			bone->name.string,
			&joint);

		if (jointTransform)
		{
			jointTransform->position = getCurrentPosition(
				bone,
				animator->time,
				animator->backwards);
			jointTransform->rotation = getCurrentRotation(
				bone,
				animator->time,
				animator->backwards);
			jointTransform->scale = getCurrentScale(
				bone,
				animator->time,
				animator->backwards);
			tMarkDirty(scene, joint);
		}
	}

	real32 direction = animator->backwards ? -1.0f : 1.0f;
	real64 deltaTime = direction * animator->speed * dt;

	animator->time += deltaTime;

	bool stopped = false;
	if ((!animator->backwards && animator->time > animator->duration) ||
		 (animator->backwards && animator->time < 0.0))
	{
		if (animator->loopCount > -1)
		{
			if (--animator->loopCount == -1)
			{
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
	listPushFront(&system.componentTypes, &nextAnimationComponentID);

	system.run = &runAnimationSystem;

	return system;
}

kmVec3 getCurrentPosition(Bone *bone, real64 time, bool backwards)
{
	if (bone->numPositionKeyFrames == 1)
	{
		return bone->positionKeyFrames[0].value;
	}

	uint32 a = backwards ? bone->numPositionKeyFrames - 2 : 0;
	uint32 b = a + 1;

	for (uint32 i = 0; i < bone->numPositionKeyFrames - 1; i++)
	{
		if (time < bone->positionKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	Vec3KeyFrame *keyFrameA = &bone->positionKeyFrames[backwards ? b : a];
	Vec3KeyFrame *keyFrameB = &bone->positionKeyFrames[backwards ? a : b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 position;
	kmVec3Lerp(&position, &keyFrameA->value, &keyFrameB->value, t);

	return position;
}

kmQuaternion getCurrentRotation(Bone *bone, real64 time, bool backwards)
{
	if (bone->numRotationKeyFrames == 1)
	{
		return bone->rotationKeyFrames[0].value;
	}

	uint32 a = backwards ? bone->numPositionKeyFrames - 2 : 0;
	uint32 b = a + 1;

	for (uint32 i = 0; i < bone->numRotationKeyFrames - 1; i++)
	{
		if (time < bone->rotationKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	QuaternionKeyFrame *keyFrameA = &bone->rotationKeyFrames[backwards ? b : a];
	QuaternionKeyFrame *keyFrameB = &bone->rotationKeyFrames[backwards ? a : b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmQuaternion rotation;
	quaternionSlerp(&rotation, &keyFrameA->value, &keyFrameB->value, t);

	return rotation;
}

kmVec3 getCurrentScale(Bone *bone, real64 time, bool backwards)
{
	if (bone->numScaleKeyFrames == 1)
	{
		return bone->scaleKeyFrames[0].value;
	}

	uint32 a = backwards ? bone->numPositionKeyFrames - 2 : 0;
	uint32 b = a + 1;

	for (uint32 i = 0; i < bone->numScaleKeyFrames - 1; i++)
	{
		if (time < bone->scaleKeyFrames[i + 1].time)
		{
			a = i, b = i + 1;
			break;
		}
	}

	Vec3KeyFrame *keyFrameA = &bone->scaleKeyFrames[backwards ? b : a];
	Vec3KeyFrame *keyFrameB = &bone->scaleKeyFrames[backwards ? a : b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 scale;
	kmVec3Lerp(&scale, &keyFrameA->value, &keyFrameB->value, t);

	return scale;
}