#include "components/animator.h"

#include "asset_management/model.h"
#include "asset_management/animation.h"

#include "data/data_types.h"
#include "data/hash_map.h"

HashMap animationReferences;

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	int32 loopCount,
	real32 speed,
	real32 transitionDuration,
	bool stopPreviousAnimation)
{
	if (strlen(name) == 0 ||
		!strcmp(animator->currentAnimation, name) ||
		(animator->transitionTime < animator->transitionDuration
			&& strlen(animator->currentAnimation) > 0))
	{
		return;
	}

	if (stopPreviousAnimation)
	{
		stopAnimation(animator);
	}

	AnimationReference *animationReference = setCurrentAnimationReference(
		modelComponent,
		animator,
		name);

	if (!animationReference)
	{
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

AnimationReference* setCurrentAnimationReference(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name)
{
	AnimationReference *animationReference = hashMapGetData(
		animationReferences,
		&animator);

	if (!modelComponent)
	{
		resetAnimator(animator, animationReference);
		return NULL;
	}

	Model model = getModel(modelComponent->name);
	if (strlen(model.name.string) == 0)
	{
		return NULL;
	}

	animationReference->currentAnimation = getAnimation(&model, name);
	if (!animationReference->currentAnimation)
	{
		resetAnimator(animator, animationReference);
		return NULL;
	}

	return animationReference;
}

void stopAnimation(AnimatorComponent *animator)
{
	if (animator->transitionTime < animator->transitionDuration &&
		strlen(animator->currentAnimation) > 0)
	{
		return;
	}

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
	if (animationReference)
	{
		animationReference->currentAnimation = NULL;
	}

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