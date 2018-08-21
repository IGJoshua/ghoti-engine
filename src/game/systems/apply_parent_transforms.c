#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"

#include <kazmath/mat3.h>
#include <kazmath/mat4.h>
#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

#include <string.h>

internal UUID transformComponentID = {};
internal UUID emptyID = {};

internal
kmMat4 composeTransform(
	const kmVec3 *position,
	const kmQuaternion *rotation,
	const kmVec3 *scale)
{
	kmMat3 rotationMatrix;
	kmMat3FromRotationQuaternion(&rotationMatrix, rotation);

	kmMat4 transform;
	kmMat4RotationTranslation(&transform, &rotationMatrix, position);

	kmMat4 scaleMatrix;
	kmMat4Scaling(
		&scaleMatrix,
		scale->x,
		scale->y,
		scale->z);

	kmMat4Multiply(&transform, &transform, &scaleMatrix);

	return transform;
}

internal
void decomposeTransform(
	const kmMat4 *transform,
	kmVec3 *outPosition,
	kmQuaternion *outRotation,
	kmVec3 *outScale)
{
	kmMat4ExtractTranslationVec3(transform, outPosition);

	kmMat3 rotationMatrix;
	kmMat4ExtractRotationMat3(transform, &rotationMatrix);

	kmQuaternionRotationMatrix(outRotation, &rotationMatrix);

	kmVec3 xAxis;
	kmVec3Fill(&xAxis, transform->mat[0], transform->mat[1], transform->mat[2]);

	kmVec3 yAxis;
	kmVec3Fill(&yAxis, transform->mat[4], transform->mat[5], transform->mat[6]);

	kmVec3 zAxis;
	kmVec3Fill(
		&zAxis,
		transform->mat[8],
		transform->mat[9],
		transform->mat[10]);

	kmVec3Fill(
		outScale,
		kmVec3Length(&xAxis),
		kmVec3Length(&yAxis),
		kmVec3Length(&zAxis)
	);
}

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

		kmVec3 tempPos;
		kmVec3Mul(
			&tempPos,
			&outTransform->position,
			&parentTransform->globalScale);
		kmQuaternionMultiplyVec3(
			&tempPos,
			&parentTransform->globalRotation,
			&tempPos);
		kmVec3Add(
			&outTransform->globalPosition,
			&parentTransform->globalPosition,
			&tempPos);

		kmQuaternionMultiply(
			&outTransform->globalRotation,
			&parentTransform->globalRotation,
			&outTransform->rotation);

		kmMat4 parentTransformMatrix = composeTransform(
			&parentTransform->globalPosition,
			&parentTransform->globalRotation,
			&parentTransform->globalScale);
		kmMat4 outTransformMatrix = composeTransform(
			&outTransform->position,
			&outTransform->rotation,
			&outTransform->scale);

		kmMat4 transformMatrix;
		kmMat4Multiply(
			&transformMatrix,
			&parentTransformMatrix,
			&outTransformMatrix);

		kmQuaternion tempRot;
		decomposeTransform(
			&transformMatrix,
			&tempPos,
			&tempRot,
			&outTransform->globalScale);
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