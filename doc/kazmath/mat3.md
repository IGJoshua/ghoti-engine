### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmMat3

# kmMat3

A structure that represents a 3x3 matrix
```c
typedef struct kmMat3{
    kmScalar mat[9];
} kmMat3;
```
```
    | 0  3  6 |
m = | 1  4  7 |
    | 2  5  8 |
```
## kmMat3* kmMat3Fill
Returns `pOut` filled with `pMat`
```c
kmMat3* kmMat3Fill(kmMat3* pOut, const kmScalar* pMat);
```

## kmMat3* kmMat3Adjugate
Returns the adjugate(a.k.a. classical adjoint, or adjunct) of `pIn` (the transpose of its cofactor matrix).
```c
kmMat3* kmMat3Adjugate(kmMat3* pOut, const kmMat3* pIn);
```
## kmMat3* kmMat3Identity
Sets `pOut` to an identity matrix returns `pOut`
```c
kmMat3* kmMat3Identity(kmMat3* pOut);
```
## kmBool kmMat3IsIdentity
Returns true if `pIn` is an identity matrix
```c
kmBool kmMat3IsIdentity(const kmMat3* pIn);
```
## kmMat3* kmMat3Inverse
Returns the inverse of `pM`
```c
kmMat3* kmMat3Inverse(kmMat3* pOut, const kmMat3* pM);
```
## kmMat3* kmMat3Transpose
Sets `pOut` to the transpose of `pIn`, returns `pOut`
```c
kmMat3* kmMat3Transpose(kmMat3* pOut, const kmMat3* pIn);
```
## kmScalar kmMat3Determinant
Returns the determinant of `pIn`
```c
kmScalar kmMat3Determinant(const kmMat3* pIn);
```
## kmBool kmMat3AreEqual
Returns true if the 2 matrices are equal (approximately)
```c
kmBool kmMat3AreEqual(const kmMat3* pMat1, const kmMat3* pMat2);
```
## kmMat3* kmMat3AssignMat3
Assigns the value of `pIn` to `pOut`
```c
kmMat3* kmMat3AssignMat3(kmMat3* pOut, const kmMat3* pIn);
```

## kmMat3* kmMat3MultiplyMat3
Multiplies `lhs` (matrix) with `rhs` (matrix), stores the result in `pOut`, returns `pOut`
```c
kmMat3* kmMat3MultiplyMat3(kmMat3* pOut, const kmMat3* lhs, const kmMat3* rhs);
```

## kmMat3* kmMat3MultiplyScalar
Multiplies `lhs` (matrix) with `rhs` (scalar), stores the result in `pOut`, returns `pOut`
```c
kmMat3* kmMat3MultiplyScalar(kmMat3* pOut, const kmMat3* lhs,
                             const kmScalar rhs);
```                             

## kmMat3* kmMat3FromRotation
Builds a rotation matrix around the X, Y or Z axis and stores it in `pOut`, returns `pOut`  
Scalable by radians or degrees (kmMat3FromRotation_InDegrees).
```c
kmMat3* kmMat3FromRotationX(kmMat3* pOut, const kmScalar radians);
kmMat3* kmMat3FromRotationY(kmMat3* pOut, const kmScalar radians);
kmMat3* kmMat3FromRotationZ(kmMat3* pOut, const kmScalar radians);
```
```c
kmMat3* kmMat3FromRotationXInDegrees(kmMat3* pOut, const kmScalar degrees);
kmMat3* kmMat3FromRotationYInDegrees(kmMat3* pOut, const kmScalar degrees);
kmMat3* kmMat3FromRotationZInDegrees(kmMat3* pOut, const kmScalar degrees);
```

## kmMat3* kmMat3FromRotationQuaternion
Builds a rotation matrix from a quaternion and stores it in `pOut`, returns `pOut`  
```c
kmMat3* kmMat3FromRotationQuaternion(kmMat3* pOut,
                                     const struct kmQuaternion* quaternion);
```

## kmMat3* kmMat3FromRotationLookAt
Builds a rotation matrix and stores it in `pOut`, returns `pOut`  
* `pEye` is where the new matrix will be facing  
* `pCentre` is the center point of the matrix  
* `pUp` is the direction of up.
```c
kmMat3* kmMat3FromRotationLookAt(kmMat3* pOut, const struct kmVec3* pEye,
                                 const struct kmVec3* pCentre,
                                 const struct kmVec3* pUp);
```

## kmMat3* kmMat3FromScaling
Builds a scaling matrix
```c
kmMat3* kmMat3FromScaling(kmMat3* pOut, const kmScalar x, const kmScalar y);
```

## kmMat3* kmMat3FromTranslation
Builds a translation matrix
```c
kmMat3* kmMat3FromTranslation(kmMat3* pOut, const kmScalar x, const kmScalar y);
```

## kmMat3* kmMat3FromRotationAxisAngle
Builds a matrix from an axis-angle
```c
kmMat3* kmMat3FromRotationAxisAngle(kmMat3* pOut, const struct kmVec3* axis, const kmScalar radians);
```
```c
kmMat3* kmMat3FromRotationAxisAngleInDegrees(kmMat3* pOut, const struct kmVec3* axis, const kmScalar degrees);
```
## void kmMat3ExtractRotationAxisAngle
Extracts an axis-angle from `self`
```c
void kmMat3ExtractRotationAxisAngle(const kmMat3* self, struct kmVec3* axis, kmScalar* radians);
void kmMat3ExtractRotationAxisAngleInDegrees(const kmMat3* self, struct kmVec3* axis, kmScalar* degrees);
```
## kmMat3ExtractUpVec3 / kmMat3ExtractRightVec3 / kmMat3ExtractForwardVec3
Extracts a vector from `self`
```c
kmVec3* kmMat3ExtractUpVec3(const kmMat3* self, struct kmVec3* pOut);
kmVec3* kmMat3ExtractRightVec3(const kmMat3* self, struct kmVec3* pOut);
kmVec3* kmMat3ExtractForwardVec3(const kmMat3* self, struct kmVec3* pOut);
```
