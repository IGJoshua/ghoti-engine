### [Table of Contents](../../main.md) -> [Lua](../Lua.md)  -> [Kazmath](kazmath.md) -> kmQuaternion

# kmQuaternion
A structure that represents a quaternion
```c
typedef struct kmQuaternion {
	kmScalar x;
	kmScalar y;
	kmScalar z;
	kmScalar w;
} kmQuaternion;
```
## int kmQuaternionAreEqual
Returns `KM_TRUE` if the quaternions are equal, otherwise `KM_FALSE`
```c
int kmQuaternionAreEqual(const kmQuaternion* p1, const kmQuaternion* p2);
```
## kmQuaternion* kmQuaternionFill
Fills the quaternion with the values entered
```c
kmQuaternion* kmQuaternionFill(kmQuaternion* pOut, kmScalar x, kmScalar y,
                               kmScalar z, kmScalar w);
```
## kmScalar kmQuaternionDot
Returns the dot product of the 2 quaternion
```c
kmScalar kmQuaternionDot(const kmQuaternion* q1, const kmQuaternion* q2);
```
## kmQuaternion* kmQuaternionIdentity
Makes the passed quaternion an identity quaternion
```c
kmQuaternion* kmQuaternionIdentity(kmQuaternion* pOut);
```
## kmQuaternion* kmQuaternionInverse
Returns the inverse of the passed Quaternion
```c
kmQuaternion* kmQuaternionInverse(kmQuaternion* pOut, const kmQuaternion* pIn);
```
## int kmQuaternionIsIdentity
Returns true if the quaternion is an identity quaternion
```c
int kmQuaternionIsIdentity(const kmQuaternion* pIn);
```
## kmScalar kmQuaternionLength
Returns the length of the quaternion
```c
kmScalar kmQuaternionLength(const kmQuaternion* pIn);
```
## kmScalar kmQuaternionLengthSq
Returns the length of the quaternion squared (prevents a sqrt)
```c
kmScalar kmQuaternionLengthSq(const kmQuaternion* pIn);
```
## kmQuaternion* kmQuaternionLn
Returns the natural logarithm
```c
kmQuaternion* kmQuaternionLn(kmQuaternion* pOut, const kmQuaternion* pIn);
```
## kmQuaternion* kmQuaternionMultiply
Multiplies 2 quaternions together
```c
kmQuaternion* kmQuaternionMultiply(kmQuaternion* pOut, const kmQuaternion* q1,
                                   const kmQuaternion* q2);
```
## kmQuaternion* kmQuaternionNormalize
Normalizes a quaternion
```c
kmQuaternion* kmQuaternionNormalize(kmQuaternion* pOut,
                                    const kmQuaternion* pIn);
```
## kmQuaternion* kmQuaternionRotationAxisAngle
Rotates a quaternion around an axis
```c
kmQuaternion* kmQuaternionRotationAxisAngle(kmQuaternion* pOut,
                                            const struct kmVec3* pV,
                                            kmScalar angle);
```
## kmQuaternion* kmQuaternionRotationMatrix
Creates a quaternion from a rotation matrix
```c
kmQuaternion* kmQuaternionRotationMatrix(kmQuaternion* pOut,
                                         const struct kmMat3* pIn);
```
## kmQuaternion* kmQuaternionRotationPitchYawRoll
Create a quaternion from yaw, pitch and roll
```c
kmQuaternion* kmQuaternionRotationPitchYawRoll(kmQuaternion* pOut,
                                               kmScalar pitch,
                                               kmScalar yaw, kmScalar roll);
```
## kmQuaternion* kmQuaternionSlerp
Interpolate between 2 quaternions
```c
kmQuaternion* kmQuaternionSlerp(kmQuaternion* pOut, const kmQuaternion* q1,
                                const kmQuaternion* q2, kmScalar t);
```
## void kmQuaternionToAxisAngle
Get the axis and angle of rotation from a quaternion
```c
void kmQuaternionToAxisAngle(const kmQuaternion* pIn, struct kmVec3* pVector,
                             kmScalar* pAngle);
```
## kmQuaternion* kmQuaternionScale
Scale a quaternion
```c
kmQuaternion* kmQuaternionScale(kmQuaternion* pOut, const kmQuaternion* pIn,
                                kmScalar s);
```
## kmQuaternion* kmQuaternionAssign
Assigns `pOut`'s values to `pIn`'s values
```c
kmQuaternion* kmQuaternionAssign(kmQuaternion* pOut, const kmQuaternion* pIn);
```
## kmQuaternion* kmQuaternionAdd
Adds quaternions
```c
kmQuaternion* kmQuaternionAdd(kmQuaternion* pOut, const kmQuaternion* pQ1,
                              const kmQuaternion* pQ2);
```
## kmQuaternion* kmQuaternionSubtract
Subtracts quaternions
```c
kmQuaternion* kmQuaternionSubtract(kmQuaternion* pOut, const kmQuaternion* pQ1,
                                   const kmQuaternion* pQ2);
```
## struct kmVec3* kmQuaternionMultiplyVec3
Multiplies quaternions
```c
struct kmVec3* kmQuaternionMultiplyVec3(struct kmVec3* pOut,
                                        const kmQuaternion* q,
                                        const struct kmVec3* v);
```

## kmQuaternion* kmQuaternionRotationBetweenVec3
  Gets the shortest arc quaternion to rotate this vector to the
  destination vector.

 If you call this with a dest vector that is close to the inverse of
 this vector, we will rotate 180 degrees around the 'fallbackAxis'
 (if specified, or a generated axis if not) since in this case ANY
 axis of rotation is valid.
 ```c
kmQuaternion* kmQuaternionRotationBetweenVec3(kmQuaternion* pOut,
                                              const struct kmVec3* vec1,
                                              const struct kmVec3* vec2,
                                              const struct kmVec3* fallback);
```
## kmVec3* kmQuaternionGetUpVec3
Extract the Up vector from a quaternion. The result is stored in pOut. Returns pOut.
```c
kmVec3* kmQuaternionGetUpVec3(kmVec3* pOut, const kmQuaternion* pIn);
```
## kmVec3* kmQuaternionGetUpVec3
Extract the Right vector from a quaternion. The result is stored in pOut. Returns pOut.
```c
kmVec3* kmQuaternionGetRightVec3(kmVec3* pOut, const kmQuaternion* pIn);
```
## kmVec3* kmQuaternionGetForwardVec3RH / kmVec3* kmQuaternionGetForwardVec3LH
Extract the forward vector from a quaternion. The result is stored in pOut. Returns pOut.
```c
kmVec3* kmQuaternionGetForwardVec3RH(kmVec3* pOut, const kmQuaternion* pIn);
kmVec3* kmQuaternionGetForwardVec3LH(kmVec3* pOut, const kmQuaternion* pIn);
```
## kmScalar kmQuaternionGetPitch
Returns the pitch scalar from a quaternion.
```c
kmScalar kmQuaternionGetPitch(const kmQuaternion* q);
```
## kmScalar kmQuaternionGetYaw
Returns the yaw scalar from a quaternion.
```c
kmScalar kmQuaternionGetYaw(const kmQuaternion* q);
```
## kmScalar kmQuaternionGetRoll
Returns the roll scalar from a quaternion.
```c
kmScalar kmQuaternionGetRoll(const kmQuaternion* q);
```
## kmQuaternion* kmQuaternionLookRotation
Builds a translation quaternion
```c
kmQuaternion* kmQuaternionLookRotation(kmQuaternion* pOut,
                                       const kmVec3* direction,
                                       const kmVec3* up);
```
## kmQuaternion* kmQuaternionExtractRotationAroundAxis
Given a quaternion, and an axis. This extracts the rotation around
the axis into pOut as another quaternion. Uses the swing-twist
decomposition.
```c
kmQuaternion* kmQuaternionExtractRotationAroundAxis(const kmQuaternion* pIn,
                                                    const kmVec3* axis,
                                                    kmQuaternion* pOut);
```
## kmQuaternion* kmQuaternionBetweenVec3
Returns a Quaternion representing the angle between two vectors
```c
kmQuaternion* kmQuaternionBetweenVec3(kmQuaternion* pOut, const kmVec3* v1,
                                      const kmVec3* v2);
```
