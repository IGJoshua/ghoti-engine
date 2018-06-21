###[Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmVec4

#kmVec4
A structure that represents a vector with 4 elements
```c
typedef struct kmVec4
{
	kmScalar x;
	kmScalar y;
	kmScalar z;
	kmScalar w;
} kmVec4;
```
##kmVec4* kmVec4Fill
Fill a kmVec3 structure using 3 floating point values. The result is store in `pOut`, returns `pOut`
```c
kmVec4* kmVec4Fill(kmVec4* pOut, kmScalar x, kmScalar y, kmScalar z,
                   kmScalar w);
```
##kmVec4* kmVec4Add
 Adds 2 4D vectors together. The result is store in `pOut`, the
function returns `pOut` so that it can be nested in another
function.
```c
kmVec4* kmVec4Add(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);
```
##kmScalar kmVec4Dot
Returns the dot product of 2 4D vectors
```c
kmScalar kmVec4Dot(const kmVec4* pV1, const kmVec4* pV2);
```
##kmScalar kmVec4Length
Returns the length of a 4D vector, this uses a sqrt so if the
squared length will do use
```c
kmScalar kmVec4Length(const kmVec4* pIn);
```
##kmScalar kmVec4LengthSq
Returns the length of the 4D vector squared.
```c
kmScalar kmVec4LengthSq(const kmVec4* pIn);
```
##kmVec4* kmVec4Lerp
Returns the interpolation of 2 4D vectors based on `t`.
```c
kmVec4* kmVec4Lerp(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2,
                   kmScalar t);
```
##kmVec4* kmVec4Normalize
Normalizes a 4D vector. The result is stored in `pOut`. `pOut` is returned
```c
kmVec4* kmVec4Normalize(kmVec4* pOut, const kmVec4* pIn);
```
##kmVec4* kmVec4Scale
Scales a vector to the required length. This performs a Normalize
before multiplying by `s`.
```c
kmVec4* kmVec4Scale(kmVec4* pOut, const kmVec4* pIn, const kmScalar s);
```
##kmVec4* kmVec4Subtract
Subtracts one 4D `pV2` from `pV1`. The result is stored in `pOut`. `pOut`
is returned
```c
kmVec4* kmVec4Subtract(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);
```
##kmVec4* kmVec4Mul
Multiplies 4D `pV1` and `pV2`. The result is stored in `pOut`. `pOut`
is returned
```c
kmVec4* kmVec4Mul( kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2 );
```
##kmVec4* kmVec4Div
Divides 4D `pV1` by `pV2`. The result is stored in `pOut`. `pOut`
is returned
```c
kmVec4* kmVec4Div( kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2 );
```
##kmVec4* kmVec4MultiplyMat4
Multiplies a 4D vector by a matrix, the result is stored in `pOut`,
and `pOut` is returned.
```c
kmVec4* kmVec4MultiplyMat4(kmVec4* pOut, const kmVec4* pV,
                           const struct kmMat4* pM);
```
##kmVec4* kmVec4Transform
Transforms the vector by translation, rotation, and scale of the matrix
```c
kmVec4* kmVec4Transform(kmVec4* pOut, const kmVec4* pV,
                        const struct kmMat4* pM);
```
##kmVec4* kmVec4TransformArray
Loops through an input array transforming each vec4 by the
matrix.
```c
kmVec4* kmVec4TransformArray(kmVec4* pOut, unsigned int outStride,
                            const kmVec4* pV, unsigned int vStride,
                            const struct kmMat4* pM, unsigned int count); 
```
##int kmVec4AreEqual
Returns `KM_TRUE` if the 2 vectors are approximately equal
```c
int kmVec4AreEqual(const kmVec4* p1, const kmVec4* p2);
```
##kmVec4* kmVec4Assign
Assigns `pIn` to `pOut`. Returns `pOut`. If `pIn` and `pOut` are the same
then nothing happens but `pOut` is still returned
```c
kmVec4* kmVec4Assign(kmVec4* pOut, const kmVec4* pIn);
```
##void kmVec4Swap
Swaps the values in one vector with another.
```c
void kmVec4Swap(kmVec4* pA, kmVec4* pB);
```
