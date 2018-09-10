#include "defines.h"

#include "asset_management/asset_manager_types.h"

#include "components/component_types.h"

typedef struct animation_reference_t
{
	Animation *previousAnimation;
	Animation *currentAnimation;
} AnimationReference;

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	int32 loopCount,
	real32 speed,
	real32 transitionDuration,
	bool stopPreviousAnimation);
AnimationReference* setCurrentAnimationReference(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name);
void stopAnimation(AnimatorComponent *animator);
void resetAnimator(
	AnimatorComponent *animator,
	AnimationReference *animationReference);
void removeAnimator(AnimatorComponent *animator);