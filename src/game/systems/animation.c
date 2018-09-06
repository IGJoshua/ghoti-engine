#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"

#include "components/component_types.h"
#include "components/animation.h"
#include "components/transform.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "math/math.h"

internal UUID modelComponentID = {};
internal UUID animationComponentID = {};

internal kmVec3 getCurrentPosition(Bone *bone, real64 time, bool backwards);
internal kmQuaternion getCurrentRotation(
	Bone *bone,
	real64 time,
	bool backwards);
internal kmVec3 getCurrentScale(Bone *bone, real64 time, bool backwards);

internal void runAnimationSystem(Scene *scene, UUID entityID, real64 dt)
{
	AnimationComponent *animationComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		animationComponentID);
	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	Model *model = getModel(modelComponent->name);

	if (!model)
	{
		stopAnimation(animationComponent);
		return;
	}

	Animation *animation = NULL;
	for (uint32 i = 0; i < model->numAnimations; i++)
	{
		if (!strcmp(
			model->animations[i].name.string,
			animationComponent->name))
		{
			animation = &model->animations[i];
			break;
		}
	}

	if (!animation)
	{
		stopAnimation(animationComponent);
		return;
	}

	if (animationComponent->paused)
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
				animationComponent->time,
				animationComponent->backwards);
			jointTransform->rotation = getCurrentRotation(
				bone,
				animationComponent->time,
				animationComponent->backwards);
			jointTransform->scale = getCurrentScale(
				bone,
				animationComponent->time,
				animationComponent->backwards);
			tMarkDirty(scene, joint);
		}
	}

	real32 direction = animationComponent->backwards ? -1.0f : 1.0f;
	real64 deltaTime = direction * animationComponent->speed * dt;

	animationComponent->time += deltaTime;

	if (animationComponent->loop)
	{
		while (animationComponent->time < 0.0)
		{
			animationComponent->time += animationComponent->duration;
		}

		animationComponent->time = fmod(
			animationComponent->time,
			animationComponent->duration);
	}
	else if (
		(animationComponent->backwards && animationComponent->time < 0.0) || (!animationComponent->backwards &&
			animationComponent->time > animationComponent->duration))
	{
		stopAnimation(animationComponent);
	}
}

System createAnimationSystem(void)
{
	System system = {};

	modelComponentID = idFromName("model");
	animationComponentID = idFromName("animation");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &modelComponentID);
	listPushFront(&system.componentTypes, &animationComponentID);

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