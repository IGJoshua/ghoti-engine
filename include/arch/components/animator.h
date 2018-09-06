#include "defines.h"

#include "components/component_types.h"

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	bool loop,
	real32 speed,
	bool backwards);
void stopAnimation(AnimatorComponent *animator);