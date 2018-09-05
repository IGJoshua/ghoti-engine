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

internal kmVec3 getCurrentPosition(Bone *bone, real64 time, real64 duration);
internal kmQuaternion getCurrentRotation(
	Bone *bone,
	real64 time,
	real64 duration);
internal kmVec3 getCurrentScale(Bone *bone, real64 time, real64 duration);

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
		return;
	}

	animationComponent->time = fmod(
		animationComponent->time + dt,
		animation->duration);

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
				animation->duration);
			jointTransform->rotation = getCurrentRotation(
				bone,
				animationComponent->time,
				animation->duration);
			jointTransform->scale = getCurrentScale(
				bone,
				animationComponent->time,
				animation->duration);
			tMarkDirty(scene, joint);
		}
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

kmVec3 getCurrentPosition(Bone *bone, real64 time, real64 duration)
{
	if (bone->numPositionKeyFrames == 1)
	{
		return bone->positionKeyFrames[0].value;
	}

	int32 a = -1;
	for (uint32 i = 0; i < bone->numPositionKeyFrames - 1; i++)
	{
		if (time < bone->positionKeyFrames[i + 1].time)
		{
			a = i;
			break;
		}
	}

	int32 b = a + 1;
	if (a == -1)
	{
		a = bone->numPositionKeyFrames - 1;
		b = 0;
	}

	Vec3KeyFrame *keyFrameA = &bone->positionKeyFrames[a];
	Vec3KeyFrame *keyFrameB = &bone->positionKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	if (b == 0)
	{
		dt = duration - keyFrameA->time;
	}

	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 position;
	kmVec3Lerp(&position, &keyFrameA->value, &keyFrameB->value, t);

	return position;
}

kmQuaternion getCurrentRotation(Bone *bone, real64 time, real64 duration)
{
	if (bone->numRotationKeyFrames == 1)
	{
		return bone->rotationKeyFrames[0].value;
	}

	int32 a = -1;
	for (uint32 i = 0; i < bone->numRotationKeyFrames - 1; i++)
	{
		if (time < bone->rotationKeyFrames[i + 1].time)
		{
			a = i;
			break;
		}
	}

	int32 b = a + 1;
	if (a == -1)
	{
		a = bone->numRotationKeyFrames - 1;
		b = 0;
	}

	QuaternionKeyFrame *keyFrameA = &bone->rotationKeyFrames[a];
	QuaternionKeyFrame *keyFrameB = &bone->rotationKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	if (b == 0)
	{
		dt = duration - keyFrameA->time;
	}

	real64 t = (time - keyFrameA->time) / dt;

	kmQuaternion rotation;
	quaternionSlerp(&rotation, &keyFrameA->value, &keyFrameB->value, t);

	return rotation;
}

kmVec3 getCurrentScale(Bone *bone, real64 time, real64 duration)
{
	if (bone->numScaleKeyFrames == 1)
	{
		return bone->scaleKeyFrames[0].value;
	}

	int32 a = -1;
	for (uint32 i = 0; i < bone->numScaleKeyFrames - 1; i++)
	{
		if (time < bone->scaleKeyFrames[i + 1].time)
		{
			a = i;
			break;
		}
	}

	int32 b = a + 1;
	if (a == -1)
	{
		a = bone->numScaleKeyFrames - 1;
		b = 0;
	}

	Vec3KeyFrame *keyFrameA = &bone->scaleKeyFrames[a];
	Vec3KeyFrame *keyFrameB = &bone->scaleKeyFrames[b];

	real64 dt = keyFrameB->time - keyFrameA->time;
	if (b == 0)
	{
		dt = duration - keyFrameA->time;
	}

	real64 t = (time - keyFrameA->time) / dt;

	kmVec3 scale;
	kmVec3Lerp(&scale, &keyFrameA->value, &keyFrameB->value, t);

	return scale;
}