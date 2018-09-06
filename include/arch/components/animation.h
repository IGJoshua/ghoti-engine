#include "defines.h"

#include "components/component_types.h"

#include "ECS/scene.h"

TransformComponent* getJointTransform(
	Scene *scene,
	UUID joint,
	const char *name,
	UUID *uuid);

void playAnimation(
	ModelComponent *modelComponent,
	AnimationComponent *animationComponent,
	const char *name,
	bool loop,
	real32 speed,
	bool backwards);
void stopAnimation(AnimationComponent *animationComponent);