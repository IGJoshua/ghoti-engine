#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"

internal UUID transformComponentID = {};

internal
void initCleanGlobalTransformsSystem(Scene *scene)
{
	ComponentDataTable **table = hashMapGetData(
		scene->componentTypes,
		&transformComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(*table);
		!cdtIteratorAtEnd(itr);
		cdtMoveIterator(&itr))
	{
		TransformComponent *transform = cdtIteratorGetData(itr);

		transform->lastGlobalPosition = transform->globalPosition;
		transform->lastGlobalRotation = transform->globalRotation;
		transform->lastGlobalScale = transform->globalScale;
		kmQuaternionNormalize(
			&transform->globalRotation,
			&transform->globalRotation);
		kmQuaternionNormalize(
			&transform->lastGlobalRotation,
			&transform->lastGlobalRotation);
	}
}

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
	kmQuaternionNormalize(
		&transform->globalRotation,
		&transform->globalRotation);
	kmQuaternionNormalize(
		&transform->lastGlobalRotation,
		&transform->lastGlobalRotation);
}

System createCleanGlobalTransformsSystem(void)
{
	transformComponentID = idFromName("transform");

	System cleanGlobalTransforms = {};

	cleanGlobalTransforms.componentTypes = createList(sizeof(UUID));
	listPushFront(&cleanGlobalTransforms.componentTypes, &transformComponentID);

	cleanGlobalTransforms.init = &initCleanGlobalTransformsSystem;
	cleanGlobalTransforms.begin = 0;
	cleanGlobalTransforms.run = &runCleanGlobalTransformsSystem;
	cleanGlobalTransforms.end = 0;
	cleanGlobalTransforms.shutdown = 0;

	return cleanGlobalTransforms;
}
