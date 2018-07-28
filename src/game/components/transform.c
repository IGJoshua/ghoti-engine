#include "components/transform.h"

void tMarkDirty(Scene *scene, UUID entityID)
{
	UUID transformComponentID = idFromName("transform");

	TransformComponent *trans =
		sceneGetComponentFromEntity(scene, entityID, transformComponentID);

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
				transformComponentID);

			tMarkDirty(scene, currentChild);
		}
	}
}

void tDecomposeMat4(
	kmMat4 const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale)
{
	// TODO: make a way to get the stuff out of the matrix
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
