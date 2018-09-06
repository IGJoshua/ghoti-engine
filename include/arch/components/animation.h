#include "defines.h"

#include "components/component_types.h"

#include "ECS/ecs_types.h"

TransformComponent* getJointTransform(
	Scene *scene,
	UUID joint,
	const char *name,
	UUID *uuid);