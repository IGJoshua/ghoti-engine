#include "components/transform.h"

#include "components/rigid_body.h"

#include <kazmath/mat3.h>

internal
void markDirtyHelper(
	Scene *scene,
	UUID *transformComponentID,
	TransformComponent *trans)
{
	if (trans)
	{
		trans->dirty = true;

		TransformComponent *child = 0;

		for (UUID currentChild = trans->firstChild;
			 strcmp(currentChild.string, "");
			 currentChild = child->nextSibling)
		{
			child = sceneGetComponentFromEntity(
				scene,
				currentChild,
				*transformComponentID);

			markDirtyHelper(
				scene,
				transformComponentID,
				child);
		}
	}
}

void tMarkDirty(Scene *scene, UUID entityID)
{
	UUID transformComponentID = idFromName("transform");

	TransformComponent *trans =
		sceneGetComponentFromEntity(scene, entityID, transformComponentID);

	if (trans)
	{
		markDirtyHelper(
			scene,
			&transformComponentID,
			trans);
	}
}

void tDecomposeMat4(
	kmMat4 const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale)
{
	kmMat4ExtractTranslationVec3(transform, position);

	kmMat3 rotationMat = {};
	kmMat4ExtractRotationMat3(transform, &rotationMat);

	kmQuaternionRotationMatrix(rotation, &rotationMat);

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
		scale,
		kmVec3Length(&xAxis),
		kmVec3Length(&yAxis),
		kmVec3Length(&zAxis));
}

kmMat4 tComposeMat4(
	kmVec3 const *position,
	kmQuaternion const *rotation,
	kmVec3 const *scale)
{
	kmQuaternion rotationQuat;
	kmQuaternionAssign(&rotationQuat, rotation);
	kmQuaternionNormalize(
		&rotationQuat,
		&rotationQuat);

	kmVec3 pos;
	kmVec3Assign(&pos, position);

	kmVec3 s;
	kmVec3Assign(&s, scale);

	kmMat4 rotationMatrix;
	kmMat4RotationQuaternion(&rotationMatrix, &rotationQuat);

	kmMat4 positionMatrix;
	kmMat4Translation(
		&positionMatrix,
		pos.x,
		pos.y,
		pos.z);

	kmMat4 scaleMatrix;
	kmMat4Scaling(
		&scaleMatrix,
		s.x,
		s.y,
		s.z);

	kmMat4 worldMatrix;
	kmMat4Multiply(
		&worldMatrix,
		&positionMatrix,
		kmMat4Multiply(&worldMatrix, &rotationMatrix, &scaleMatrix));

	return worldMatrix;
}

void tGetInterpolatedTransform(
	TransformComponent *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale,
	real64 alpha)
{
	if (kmVec3AreEqual(&transform->lastGlobalPosition, &KM_VEC3_ZERO))
	{
		kmVec3Assign(
			&transform->lastGlobalPosition,
			&transform->globalPosition);
	}

	if (kmQuaternionIsIdentity(&transform->lastGlobalRotation))
	{
		kmQuaternionAssign(
			&transform->lastGlobalRotation,
			&transform->globalRotation);
	}

	kmVec3 defaultScale;
	kmVec3Fill(&defaultScale, 1.0f, 1.0f, 1.0f);

	if (kmVec3AreEqual(&transform->lastGlobalScale, &defaultScale))
	{
		kmVec3Assign(
			&transform->lastGlobalScale,
			&transform->globalScale);
	}

	kmVec3Lerp(
		position,
		&transform->lastGlobalPosition,
		&transform->globalPosition,
		(real32)alpha);

	kmQuaternionSlerp(
		rotation,
		&transform->lastGlobalRotation,
		&transform->globalRotation,
		(real32)alpha);

	kmVec3Lerp(
		scale,
		&transform->lastGlobalScale,
		&transform->globalScale,
		(real32)alpha);
}

kmMat4 tGetInterpolatedTransformMatrix(
	TransformComponent *transform,
	real64 alpha)
{
	kmVec3 position;
	kmQuaternion rotation;
	kmVec3 scale;

	tGetInterpolatedTransform(transform, &position, &rotation, &scale, alpha);

	return tComposeMat4(&position, &rotation, &scale);
}

void tGetInverseTransform(
	TransformComponent const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale)
{
	kmVec3Scale(position, &transform->position, -1.0f);
	kmQuaternionInverse(rotation, &transform->rotation);
	scale->x = 1.0f / transform->scale.x;
	scale->y = 1.0f / transform->scale.y;
	scale->z = 1.0f / transform->scale.z;
}

void tGetInverseGlobalTransform(
	TransformComponent const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale)
{
	kmVec3Scale(position, &transform->globalPosition, -1.0f);
	kmQuaternionInverse(rotation, &transform->globalRotation);
	scale->x = 1.0f / transform->globalScale.x;
	scale->y = 1.0f / transform->globalScale.y;
	scale->z = 1.0f / transform->globalScale.z;
}

void removeTransform(Scene *scene, UUID entity, TransformComponent *transform)
{
	UUID transformComponentID = idFromName("transform");

	UUID child = transform->firstChild;
	UUID sibling = {};

	// Loop through all the children and delete them
	while (child.string[0] != 0)
	{
		TransformComponent *transformComponent = sceneGetComponentFromEntity(
			scene,
			child,
			transformComponentID);

		sibling = transformComponent->nextSibling;

		sceneRemoveEntity(scene, child);
		child = sibling;
	}

	if (transform->parent.string[0] != 0)
	{
		transform = sceneGetComponentFromEntity(
			scene,
			transform->parent,
			transformComponentID);
		TransformComponent *previousTransform = transform;
		transform = sceneGetComponentFromEntity(
			scene,
			transform->firstChild,
			transformComponentID);

		if (!strcmp(entity.string, previousTransform->firstChild.string))
		{
			previousTransform->firstChild = transform->nextSibling;
		}
		else
		{
			do
			{
				previousTransform = transform;
				sibling = transform->nextSibling;
				transform = sceneGetComponentFromEntity(
					scene,
					sibling,
					transformComponentID);

				if (!strcmp(entity.string, sibling.string))
				{
					previousTransform->nextSibling = transform->nextSibling;
					break;
				}
			} while (transform);
		}
	}
}