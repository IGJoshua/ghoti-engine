#include "defines.h"

#include "components/component_types.h"

#include "ECS/scene.h"

TransformComponent* getJointTransform(
	Scene *scene,
	UUID joint,
	const char *name);