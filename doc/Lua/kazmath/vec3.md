### [Table of Contents](../../main.md) -> [Lua](../Lua.md) -> [Kazmath](kazmath.md) -> kmVec3

# kmVec3
A structure that represents a vector with 3 elements
```c
typedef struct kmVec3 {
	kmScalar x;
	kmScalar y;
	kmScalar z;
} kmVec3;
```
## kmVec3 Constants
Can be used to assign a vector to these values
```c
kmVec3 KM_VEC3_NEG_Z =  { 0, 0, 1 };
kmVec3 KM_VEC3_POS_Z = { 0, 0, -1 };
kmVec3 KM_VEC3_POS_Y = { 0, 1, 0 };
kmVec3 KM_VEC3_NEG_Y = { 0, -1, 0 };
kmVec3 KM_VEC3_NEG_X = { -1, 0, 0 };
kmVec3 KM_VEC3_POS_X = { 1, 0, 0 };
kmVec3 KM_VEC3_ZERO = { 0, 0, 0 };
```
## kmVec3* kmVec3Fill
Fill a kmVec3 structure using 3 floating point values
The result is store in `pOut`, returns `pOut`
```c
kmVec3* kmVec3Fill(kmVec3* pOut, kmScalar x, kmScalar y, kmScalar z);
```
## kmScalar kmVec3Length
Returns the length of the vector
```c
kmScalar kmVec3Length(const kmVec3* pIn);
```
## kmScalar kmVec3LengthSq
Returns the square of the length of the vector
```c
kmScalar kmVec3LengthSq(const kmVec3* pIn);
```
## kmVec3* kmVec3Lerp
Returns the interpolation of 2 4D vectors based on `t`.
```c
kmVec3* kmVec3Lerp(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2,
                   kmScalar t);
```
## kmVec3* kmVec3Normalize
Returns the vector passed in set to unit length
the result is stored in `pOut`.
```c
kmVec3* kmVec3Normalize(kmVec3* pOut, const kmVec3* pIn);
```
## kmVec3* kmVec3Cross
Returns a vector perpendicular to 2 other vectors.
The result is stored in `pOut`.
```c
kmVec3* kmVec3Cross(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
```
## kmScalar kmVec3Dot
Returns the cosine of the angle between 2 vectors
```c
kmScalar kmVec3Dot(const kmVec3* pV1, const kmVec3* pV2);
```
## kmVec3* kmVec3Add
Adds 2 vectors and returns the result. The resulting
vector is stored in `pOut`.
```c
kmVec3* kmVec3Add(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
```
## Vec3* kmVec3Subtract
Subtracts 2 vectors and returns the result. The result is stored in
`pOut`.
```c
kmVec3* kmVec3Subtract(kmVec3* pOut, const kmVec3* pV1, const kmVec3* pV2);
```
## kmVec3* kmVec3Mul
Component-wise multiplication
```c
kmVec3* kmVec3Mul( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 );
```
## kmVec3* kmVec3Div
Component-wise division
```c
kmVec3* kmVec3Div( kmVec3* pOut,const kmVec3* pV1, const kmVec3* pV2 );
```
## kmVec3* kmVec3MultiplyMat3
Returns the result of vector * matrix multiplication in `pOut`
```c
kmVec3* kmVec3MultiplyMat3(kmVec3 *pOut, const kmVec3 *pV,
                           const struct kmMat3* pM);
```
## kmVec3* kmVec3MultiplyMat4
Multiplies vector (x, y, z, 1) by a given matrix. The result
is stored in `pOut`. `pOut` is returned.
```c
kmVec3* kmVec3MultiplyMat4(kmVec3* pOut, const kmVec3* pV,
                           const struct kmMat4* pM);
```
## kmVec3* kmVec3Transform
Transforms a vector (assuming w=1) by a given matrix (deprecated)
```c
kmVec3* kmVec3Transform(kmVec3* pOut, const kmVec3* pV1,
                        const struct kmMat4* pM);
```
## kmVec3* kmVec3TransformNormal
Transforms a 3D normal by a given matrix
```c
kmVec3* kmVec3TransformNormal(kmVec3* pOut, const kmVec3* pV,
                              const struct kmMat4* pM);
```
## kmVec3* kmVec3TransformCoord
Transforms a 3D vector by a given matrix, projecting the result
back into w = 1.
```c
kmVec3* kmVec3TransformCoord(kmVec3* pOut, const kmVec3* pV,
                             const struct kmMat4* pM);
```
## kmVec3* kmVec3Scale
Scales a vector to length `s`. Does not normalize first,
you should do that!
```c
kmVec3* kmVec3Scale(kmVec3* pOut, const kmVec3* pIn, const kmScalar s);
```
## kmBool kmVec3AreEqual
Returns `KM_TRUE` if the 2 vectors are approximately equal
```c
kmBool kmVec3AreEqual(const kmVec3* p1, const kmVec3* p2);
```
## kmVec3* kmVec3Assign
Assigns `pIn` to `pOut`. Returns `pOut`. If `pIn` and `pOut` are the same
then nothing happens but `pOut` is still returned
```c
kmVec3* kmVec3Assign(kmVec3* pOut, const kmVec3* pIn);
```
## kmVec3* kmVec3Zero
Sets all the elements of `pOut` to zero. Returns `pOut`.
```c
kmVec3* kmVec3Zero(kmVec3* pOut);
```
## kmVec3* kmVec3GetHorizontalAngle
Get the rotations that would make a (0,0,1) direction vector point
in the same direction as this direction vector.  Useful for
orienting vector towards a point.

Returns a rotation vector containing the X (pitch) and Y (raw)
rotations (in degrees) that when applied to a +Z (e.g. 0, 0, 1)
direction vector would make it point in the same direction as this
vector. The Z (roll) rotation is always 0, since two Euler
rotations are sufficient to point in any given direction.
```c
kmVec3* kmVec3GetHorizontalAngle(kmVec3* pOut, const kmVec3 *pIn);
```
## kmVec3* kmVec3RotationToDirection
Builds a direction vector from input vector.

Input vector is assumed to be rotation vector composed from 3 Euler
angle rotations, in degrees.  The forwards vector will be rotated
by the input vector
```c
kmVec3* kmVec3RotationToDirection(kmVec3* pOut, const kmVec3* pIn,
                                  const kmVec3* forwards);
```

## kmVec3* kmVec3ProjectOnToPlane / kmVec3* kmVec3ProjectOnToVec3
Projects a vector onto a plane or vec3
```c
kmVec3* kmVec3ProjectOnToPlane(kmVec3* pOut, const kmVec3* point,
                               const struct kmPlane* plane);
kmVec3* kmVec3ProjectOnToVec3(const kmVec3* pIn, const kmVec3* other,
                              kmVec3* projection);
```
## kmVec3* kmVec3Reflect
Reflects a vector about a given surface normal. The surface
normal is assumed to be of unit length.
```c
kmVec3* kmVec3Reflect(kmVec3* pOut, const kmVec3* pIn, const kmVec3* normal);
```
## void kmVec3Swap
Swaps the values in one vector with another  
NB does not return a value unlike normal
```c
void kmVec3Swap(kmVec3* a, kmVec3* b);
```
## void kmVec3OrthoNormalize
Othro-Normalizes the normal vector
```c
void kmVec3OrthoNormalize(kmVec3* normal, kmVec3* tangent);
```
## kmVec3* kmVec3InverseTransform
Transforms the vector by translation, rotation, and scale of the inverse of the matrix
```c
kmVec3* kmVec3InverseTransform(kmVec3* pOut, const kmVec3* pV,
                               const struct kmMat4* pM);
```
## kmVec3* kmVec3InverseTransformNormal
Transforms the vector by rotation, and scale of the inverse of the matrix
```c
kmVec3* kmVec3InverseTransformNormal(kmVec3* pOut, const kmVec3* pVect,
                                     const struct kmMat4* pM);
```
