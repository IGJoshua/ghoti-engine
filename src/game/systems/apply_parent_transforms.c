#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/transform.h"

#include <kazmath/mat3.h>
#include <kazmath/mat4.h>
#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

#include <string.h>

internal UUID transformComponentID = {};
internal UUID emptyID = {};

internal
void applyParentTransform(Scene *scene, TransformComponent *outTransform)
{
	if (strcmp(emptyID.string, outTransform->parent.string))
	{
		TransformComponent *parentTransform = sceneGetComponentFromEntity(
			scene,
			outTransform->parent,
			transformComponentID);

		if (parentTransform->dirty)
		{
			applyParentTransform(scene, parentTransform);
		}

		tConcatenateTransforms(parentTransform, outTransform);
	}
	else
	{
		outTransform->globalPosition = outTransform->position;
		outTransform->globalRotation = outTransform->rotation;
		outTransform->globalScale = outTransform->scale;
	}
	outTransform->dirty = false;
}

internal
void initApplyParentTransformsSystem(Scene *scene)
{
	ComponentDataTable **table = hashMapGetData(
		scene->componentTypes,
		&transformComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(*table);
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		TransformComponent *transform = cdtIteratorGetData(itr);

		if (transform->dirty)
		{
			applyParentTransform(scene, transform);
		}
	}
}

internal
void runApplyParentTransformsSystem(Scene *scene, UUID entityID, real64 dt)
{
	// Get the parent
	// If the parent is a thing, then get its global transform
	// Apply the parent's global transform to this guy

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	if (transform->dirty)
	{
		applyParentTransform(scene, transform);
	}
}

System createApplyParentTransformsSystem(void)
{
	transformComponentID = idFromName("transform");

	System applyParentTransforms;

	applyParentTransforms.componentTypes = createList(sizeof(UUID));
	listPushFront(&applyParentTransforms.componentTypes, &transformComponentID);

	applyParentTransforms.init = &initApplyParentTransformsSystem;
	applyParentTransforms.begin = 0;
	applyParentTransforms.run = &runApplyParentTransformsSystem;
	applyParentTransforms.end = 0;
	applyParentTransforms.shutdown = 0;

	return applyParentTransforms;
}