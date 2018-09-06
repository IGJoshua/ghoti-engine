#include "defines.h"

#include "components/component_types.h"

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	int32 loopCount,
	real32 speed,
	bool backwards);
void stopAnimation(AnimatorComponent *animator);