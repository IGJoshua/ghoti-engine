#pragma once
#include "defines.h"

#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

kmQuaternion* quaternionSlerp(
	kmQuaternion* pOut,
	const kmQuaternion* q1,
	const kmQuaternion* q2,
	kmScalar t);