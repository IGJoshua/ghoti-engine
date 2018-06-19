#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

internal UUID transformComponentID = {};

internal
void runCleanGlobalTransformsSystem(Scene *scene, UUID entityID, real64 dt)
{
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	transform->lastGlobalPosition = transform->globalPosition;
	transform->lastGlobalRotation = transform->globalRotation;
	transform->lastGlobalScale = transform->globalScale;
	kmQuaternionNormalize(&transform->globalRotation, &transform->globalRotation);
	kmQuaternionNormalize(&transform->lastGlobalRotation, &transform->lastGlobalRotation);
}

System createCleanGlobalTransformsSystem(void)
{
	transformComponentID = idFromName("transform");

	System cleanGlobalTransforms = {};

	cleanGlobalTransforms.componentTypes = createList(sizeof(UUID));
	listPushFront(&cleanGlobalTransforms.componentTypes, &transformComponentID);

	cleanGlobalTransforms.init = 0;
	cleanGlobalTransforms.begin = 0;
	cleanGlobalTransforms.run = &runCleanGlobalTransformsSystem;
	cleanGlobalTransforms.end = 0;
	cleanGlobalTransforms.shutdown = 0;

	return cleanGlobalTransforms;
}
