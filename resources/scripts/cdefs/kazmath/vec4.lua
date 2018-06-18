ffi.cdef[[

kmVec4* kmVec4Fill(kmVec4* pOut, kmScalar x, kmScalar y, kmScalar z,
                   kmScalar w);

/** Adds 2 4D vectors together. The result is store in pOut, the
 * function returns pOut so that it can be nested in another
 * function.*/
kmVec4* kmVec4Add(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);

/** Returns the dot product of 2 4D vectors*/
kmScalar kmVec4Dot(const kmVec4* pV1, const kmVec4* pV2);

/** Returns the length of a 4D vector, this uses a sqrt so if the
 * squared length will do use*/
kmScalar kmVec4Length(const kmVec4* pIn);

/** Returns the length of the 4D vector squared.*/
kmScalar kmVec4LengthSq(const kmVec4* pIn);

/** Returns the interpolation of 2 4D vectors based on t.*/
kmVec4* kmVec4Lerp(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2,
                   kmScalar t);

/** Normalizes a 4D vector. The result is stored in pOut. pOut is returned*/
kmVec4* kmVec4Normalize(kmVec4* pOut, const kmVec4* pIn);

/** Scales a vector to the required length. This performs a Normalize
 * before multiplying by S.*/
kmVec4* kmVec4Scale(kmVec4* pOut, const kmVec4* pIn, const kmScalar s);

/** Subtracts one 4D pV2 from pV1. The result is stored in pOut. pOut
 * is returned*/
kmVec4* kmVec4Subtract(kmVec4* pOut, const kmVec4* pV1, const kmVec4* pV2);
kmVec4* kmVec4Mul( kmVec4* pOut,const kmVec4* pV1, const kmVec4* pV2 );
kmVec4* kmVec4Div( kmVec4* pOut,const kmVec4* pV1, const kmVec4* pV2 );

/** Multiplies a 4D vector by a matrix, the result is stored in pOut,
 * and pOut is returned.*/
kmVec4* kmVec4MultiplyMat4(kmVec4* pOut, const kmVec4* pV,
                           const kmMat4* pM);
kmVec4* kmVec4Transform(kmVec4* pOut, const kmVec4* pV,
                        const kmMat4* pM);

/** Loops through an input array transforming each vec4 by the
 * matrix.*/
kmVec4* kmVec4TransformArray(kmVec4* pOut, uint32 outStride,
			const kmVec4* pV, uint32 vStride, const kmMat4* pM,
                             uint32 count);
int32 	kmVec4AreEqual(const kmVec4* p1, const kmVec4* p2);

kmVec4* kmVec4Assign(kmVec4* pOut, const kmVec4* pIn);
void kmVec4Swap(kmVec4* pA, kmVec4* pB);


]]
