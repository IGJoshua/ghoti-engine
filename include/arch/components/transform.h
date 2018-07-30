#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "component_types.h"

#include <kazmath/mat4.h>

void tMarkDirty(Scene *scene, UUID entityID);

void tDecomposeMat4(
	kmMat4 const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale);
kmMat4 tComposeMat4(
	kmVec3 const *position,
	kmQuaternion const *rotation,
	kmVec3 const *scale);

void tGetInterpolatedTransform(
	TransformComponent const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale,
	real64 alpha);
kmMat4 tGetInterpolatedTransformMatrix(
	TransformComponent const *transform,
	real64 alpha);

void tGetInverseTransform(
	TransformComponent const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale);
void tGetInverseGlobalTransform(
	TransformComponent const *transform,
	kmVec3 *position,
	kmQuaternion *rotation,
	kmVec3 *scale);
