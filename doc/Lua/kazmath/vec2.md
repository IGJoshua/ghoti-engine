### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmVec2

# kmVec2
A structure that represents a vector with 2 elements
```c
typedef struct kmVec2 {
    kmScalar x;
    kmScalar y;
} kmVec2;
```
## kmVec2 Constants
Can be used to assign a vector to these values
```c
kmVec2 KM_VEC2_POS_Y = { 0, 1 };
kmVec2 KM_VEC2_NEG_Y = { 0, -1 };
kmVec2 KM_VEC2_NEG_X = { -1, 0 };
kmVec2 KM_VEC2_POS_X = { 1, 0 };
kmVec2 KM_VEC2_ZERO = { 1, 0 };
```
## kmVec2* kmVec2Fill
Fills the vector with the specified elements
```c
kmVec2* kmVec2Fill(kmVec2* pOut, kmScalar x, kmScalar y);
```
## kmScalar kmVec2Length
Returns the length of the vector
```c
kmScalar kmVec2Length(const kmVec2* pIn);
```
## kmScalar kmVec2LengthSq
Returns the square of the length of the vector
```c
kmScalar kmVec2LengthSq(const kmVec2* pIn);
```
## kmVec2* kmVec2Normalize
Returns the vector passed in set to unit length
```c
kmVec2* kmVec2Normalize(kmVec2* pOut, const kmVec2* pIn);
```
## kmVec2* kmVec2Lerp
Returns the linearly interpolated point between two points
```c
kmVec2* kmVec2Lerp(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2,
                   kmScalar t);
```
## kmVec2* kmVec2Add
Adds 2 vectors and returns the result
```c
kmVec2* kmVec2Add(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2);
```
## kmScalar kmVec2Dot
Returns the Dot product which is the cosine of the angle between
the two vectors multiplied by their kmVec2LengthSq
```c
kmScalar kmVec2Dot(const kmVec2* pV1, const kmVec2* pV2);
```
## kmScalar kmVec2Cross
Returns the signed magnitude of the vector from a 3D cross product where the Z values are assumed to be 0.
```c
kmScalar kmVec2Cross(const kmVec2* pV1, const kmVec2* pV2);
```
## kmVec2* kmVec2Subtract
Subtracts 2 vectors and returns the result
```c
kmVec2* kmVec2Subtract(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2);
```
## kmVec2* kmVec2Mul
Component-wise multiplication
```c
kmVec2* kmVec2Mul( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 );
```
## kmVec2* kmVec2Div
Component-wise division
```c
kmVec2* kmVec2Div( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 );
```
## kmVec2* kmVec2Transform
Transform the Vector
```c
kmVec2* kmVec2Transform(kmVec2* pOut, const kmVec2* pV1,
                        const struct kmMat3* pM);
```
## kmVec2* kmVec2TransformCoord
Transforms a 2D vector by a given matrix, projecting the result
back into `w` = 1.
```c
kmVec2* kmVec2TransformCoord(kmVec2* pOut, const kmVec2* pV,
                             const struct kmMat3* pM);
```
## kmVec2* kmVec2Scale
Scales a vector to length `s`
```c
kmVec2* kmVec2Scale(kmVec2* pOut, const kmVec2* pIn, const kmScalar s);
```
## kmBool kmVec2AreEqual
Returns 1 if both vectors are equal
```c
kmBool kmVec2AreEqual(const kmVec2* p1, const kmVec2* p2);
```
## kmVec2* kmVec2Assign
Assigns `pIn` to `pOut`. Returns `pOut`. If `pIn` and `pOut` are the same
then nothing happens but `pOut` is still returned
```c
kmVec2* kmVec2Assign(kmVec2* pOut, const kmVec2* pIn);
```
## kmVec2* kmVec2RotateBy
Rotates the point anticlockwise around a center by an amount of
degrees.
```c
kmVec2* kmVec2RotateBy(kmVec2* pOut, const kmVec2* pIn, const kmScalar degrees,
                       const kmVec2* center);
```
## kmScalar kmVec2DegreesBetween
Returns the angle in degrees between the two vectors
```c
kmScalar kmVec2DegreesBetween(const kmVec2* v1, const kmVec2* v2);
```
## kmScalar kmVec2DistanceBetween
Returns the distance between the two points
```c
kmScalar kmVec2DistanceBetween(const kmVec2* v1, const kmVec2* v2);
```
## kmVec2* kmVec2MidPointBetween
Returns the point mid-way between two others
```c
kmVec2* kmVec2MidPointBetween(kmVec2* pOut, const kmVec2* v1, const kmVec2* v2);
```
## kmVec2* kmVec2Reflect
Reflects a vector about a given surface normal. The surface normal
is assumed to be of unit length.
```c
kmVec2* kmVec2Reflect(kmVec2* pOut, const kmVec2* pIn, const kmVec2* normal);
```
## void kmVec2Swap
Swaps the values in `pA` with the values in `pB`
```c
void kmVec2Swap(kmVec2* pA, kmVec2* pB);
```
