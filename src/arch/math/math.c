#include "math/math.h"

#include <stdlib.h>

kmQuaternion* quaternionSlerp(
	kmQuaternion* pOut,
	const kmQuaternion* q1,
	const kmQuaternion* q2,
	kmScalar t)
{
	kmQuaternion a, b;
	kmQuaternionAssign(&a, q1);
	kmQuaternionAssign(&b, q2);

	if (kmQuaternionDot(q1, q2) < 0.0f)
	{
		kmQuaternionScale(&a, &a, -1.0f);
	}

	return kmQuaternionSlerp(pOut, &a, &b, t);
}

real64 randomRealNumber(real64 min, real64 max)
{
	return min + (rand() / (RAND_MAX / (max - min)));
}