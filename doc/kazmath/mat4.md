### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmMat4

# kmMat4

A structure that represents a 4x4 matrix
```c
typedef struct kmMat4 {
	kmScalar mat[16];
} kmMat4;
```
```
      | 0   4   8  12 |
mat = | 1   5   9  13 |
      | 2   6  10  14 |
      | 3   7  11  15 |
```

## kmMat4* kmMat4Fill

Fills a `kmMat4` structure with the values from a 16
element array of `kmScalars`  
-`pOut` - A pointer to the destination matrix  
-`pMat` - A 16 element array of `kmScalars`  
Returns `pOut` so that the call can be nested
```c
kmMat4* kmMat4Fill(kmMat4* pOut, const kmScalar* pMat);
```

## kmMat4* kmMat4Identity
Sets `pOut` to an identity matrix returns `pOut`  
-`pOut` - A pointer to the matrix to set to identity  
Returns `pOut` so that the call can be nested  
```c
kmMat4* kmMat4Identity(kmMat4* pOut);
```
## kmMat4* kmMat4Inverse
Calculates the inverse of `pM` and stores the result in
`pOut`.
Returns NULL if there is no inverse, else `pOut`
```c
kmMat4* kmMat4Inverse(kmMat4* pOut, const kmMat4* pM);
```
## int kmMat4IsIdentity
Returns `KM_TRUE` if `pIn` is an identity matrix
`KM_FALSE` otherwise
```c
int kmMat4IsIdentity(const kmMat4* pIn);
```
## kmMat4* kmMat4Transpose(
Sets `pOut` to the transpose of `pIn`, returns `pOut`
```c
kmMat4* kmMat4Transpose(kmMat4* pOut, const kmMat4* pIn);
```
## kmMat4* kmMat4Multiply
Multiplies `pM1` with `pM2`, stores the result in `pOut`, returns `pOut`
```c
kmMat4* kmMat4Multiply(kmMat4* pOut, const kmMat4* pM1, const kmMat4* pM2);
```
## kmMat4* kmMat4Assign
Assigns the value of `pIn` to `pOut`
```c
kmMat4* kmMat4Assign(kmMat4* pOut, const kmMat4* pIn);
```
```c
kmMat4* kmMat4AssignMat3(kmMat4* pOut, const struct kmMat3* pIn);
```
## int kmMat4AreEqual
Returns `KM_TRUE` if the 2 matrices are equal (approximately)
```c
int kmMat4AreEqual(const kmMat4* pM1, const kmMat4* pM2);
```
## kmMat4* kmMat4Rotation
Builds a rotation matrix around a specified axis and stores it in `pOut`, returns `pOut`
```c
kmMat4* kmMat4RotationX(kmMat4* pOut, const kmScalar radians);
kmMat4* kmMat4RotationY(kmMat4* pOut, const kmScalar radians);
kmMat4* kmMat4RotationZ(kmMat4* pOut, const kmScalar radians);
```
## kmMat4* kmMat4RotationYawPitchRoll
Builds a rotation matrix from pitch, yaw and roll. The resulting
matrix is stored in `pOut` and `pOut` is returned
```c
kmMat4* kmMat4RotationYawPitchRoll(kmMat4* pOut, const kmScalar pitch,
                                   const kmScalar yaw, const kmScalar roll);
```
## kmMat4* kmMat4RotationQuaternion
Converts a quaternion to a rotation matrix,
the result is stored in `pOut`, returns `pOut`
```c
kmMat4* kmMat4RotationQuaternion(kmMat4* pOut, const struct kmQuaternion* pQ);
```
## kmMat4* kmMat4RotationTranslation
Build a 4x4 OpenGL transformation matrix using a 3x3 rotation matrix,
and a 3d vector representing a translation. Assign the result to `pOut`,
`pOut` is also returned.
```c
kmMat4* kmMat4RotationTranslation(kmMat4* pOut, const struct kmMat3* rotation,
                                  const struct kmVec3* translation);
```
## kmMat4* kmMat4Scaling
Builds a scaling matrix
```c
kmMat4* kmMat4Scaling(kmMat4* pOut, const kmScalar x, const kmScalar y,
                      const kmScalar z);
```
## kmMat4* kmMat4Translation
Builds a translation matrix. All other elements in the matrix
will be set to zero except for the diagonal which is set to 1.0
```c
kmMat4* kmMat4Translation(kmMat4* pOut, const kmScalar x, const kmScalar y,
                          const kmScalar z);
```
## kmVec3* kmMat4GetUpVec3
Get the up vector from a matrix. `pIn` is the matrix you
wish to extract the vector from. `pOut` is a pointer to the
kmVec3 structure that should hold the resulting vector
```c
struct kmVec3* kmMat4GetUpVec3(struct kmVec3* pOut, const kmMat4* pIn);
```
## kmVec3* kmMat4GetRightVec3
Extract the right vector from a 4x4 matrix. The result is
stored in `pOut`. Returns `pOut`.
```c
struct kmVec3* kmMat4GetRightVec3(struct kmVec3* pOut, const kmMat4* pIn);
```
## kmVec3* kmMat4GetForwardVec3RH / kmVec3* kmMat4GetForwardVec3LH
Extract the forward vector from a 4x4 matrix. The result is
stored in `pOut`. Returns `pOut`.
```c
struct kmVec3* kmMat4GetForwardVec3RH(struct kmVec3* pOut, const kmMat4* pIn);
struct kmVec3* kmMat4GetForwardVec3LH(struct kmVec3* pOut, const kmMat4* pIn);
```
## kmMat4* kmMat4PerspectiveProjection
Creates a perspective projection matrix in the
same way as gluPerspective
```c
kmMat4* kmMat4PerspectiveProjection(kmMat4* pOut, kmScalar fovY,
                                    kmScalar aspect, kmScalar zNear,
                                    kmScalar zFar);
```
## kmMat4* kmMat4OrthographicProjection
Creates an orthographic projection matrix like glOrtho
```c
kmMat4* kmMat4OrthographicProjection(kmMat4* pOut, kmScalar left,
                                     kmScalar right, kmScalar bottom,
                                     kmScalar top, kmScalar nearVal,
                                     kmScalar farVal);
```
## kmMat4* kmMat4LookAt
Builds a translation matrix in the same way as gluLookAt()
the resulting matrix is stored in pOut. pOut is returned.
```c
kmMat4* kmMat4LookAt(kmMat4* pOut, const struct kmVec3* pEye,
                     const struct kmVec3* pCenter, const struct kmVec3* pUp);
```
## kmMat4* kmMat4RotationAxisAngle
Build a rotation matrix from an axis and an angle. Result is stored in pOut.
pOut is returned.
```c
kmMat4* kmMat4RotationAxisAngle(kmMat4* pOut, const struct kmVec3* axis, kmScalar radians);
```
## kmMat3* kmMat4ExtractRotationMat3
Extract a 3x3 rotation matrix from the input 4x4 transformation.
Stores the result in `pOut`, returns `pOut`
```c
struct kmMat3* kmMat4ExtractRotationMat3(const kmMat4* pIn,
                                         struct kmMat3* pOut);
```
## kmMat3* kmMat4ExtractPlane
Extract a plane from the input 4x4 transformation.
Stores the result in `pOut`, returns `pOut`
```c
struct kmPlane* kmMat4ExtractPlane(struct kmPlane* pOut, const kmMat4* pIn,
                                   const kmEnum plane);
```
## kmVec3* kmMat4RotationToAxisAngle
Take the rotation from a 4x4 transformation matrix, and return it
as an axis and an angle (in radians). Returns the output axis.
```c
struct kmVec3* kmMat4RotationToAxisAngle(struct kmVec3* pAxis,
                                         kmScalar* radians, const kmMat4* pIn);
```
## kmVec3* kmMat4ExtractTranslationVec3
Extract a translation vector from the input 4x4 transformation.
Stores the result in `pOut`, returns `pOut`
```c
struct kmVec3* kmMat4ExtractTranslationVec3(const kmMat4* pIn,
                                            struct kmVec3* pOut);
```
