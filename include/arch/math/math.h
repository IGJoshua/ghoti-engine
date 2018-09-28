#pragma once
#include "defines.h"

#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

kmQuaternion* quaternionSlerp(
	kmQuaternion* pOut,
	const kmQuaternion* q1,
	const kmQuaternion* q2,
	kmScalar t);

real64 randomRealNumber(real64 min, real64 max);
int32 randomInteger(int32 min, int32 max);