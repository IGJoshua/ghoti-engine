#include "components/animator.h"

#include "asset_management/model.h"
#include "asset_management/animation.h"

#include "data/data_types.h"
#include "data/hash_map.h"

extern HashMap animationReferences;

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	int32 loopCount,
	real32 speed,
	real32 transitionDuration,
	bool stopPreviousAnimation)
{
	if (!strcmp(animator->currentAnimation, name))
	{
		return;
	}

	AnimationReference *animationReference = hashMapGetData(
		animationReferences,
		&animator);

	if (!modelComponent)
	{
		resetAnimator(animator, animationReference);
		return;
	}

	Model *model = getModel(modelComponent->name);

	if (!model)
	{
		resetAnimator(animator, animationReference);
		return;
	}

	if (stopPreviousAnimation)
	{
		stopAnimation(animator);
	}

	animationReference->currentAnimation = getAnimation(model, name);
	if (!animationReference->currentAnimation)
	{
		resetAnimator(animator, animationReference);
		return;
	}

	strcpy(animator->currentAnimation, name);
	animator->time = speed < 0.0f ?
		animationReference->currentAnimation->duration : 0.0;
	animator->duration = animationReference->currentAnimation->duration;
	animator->loopCount = loopCount;
	animator->speed = speed;
	animator->paused = false;
	animator->transitionDuration =
		(transitionDuration < 0.0 ? 0.0 : transitionDuration) *
		animationReference->currentAnimation->duration;
}

void stopAnimation(AnimatorComponent *animator)
{
	AnimationReference *animationReference = hashMapGetData(
		animationReferences,
		&animator);

	animationReference->previousAnimation =
		animationReference->currentAnimation;

	strcpy(animator->previousAnimation, animator->currentAnimation);
	animator->previousAnimationTime = animator->time;

	resetAnimator(animator, animationReference);
}

void resetAnimator(
	AnimatorComponent *animator,
	AnimationReference *animationReference)
{
	animationReference->currentAnimation = NULL;

	strcpy(animator->currentAnimation, "");
	animator->time = 0.0;
	animator->duration = 0.0;
	animator->loopCount = 0;
	animator->paused = false;
	animator->transitionTime = 0.0;
}

void removeAnimator(AnimatorComponent *animator)
{
	if (animationReferences)
	{
		hashMapDelete(animationReferences, &animator);
	}
}