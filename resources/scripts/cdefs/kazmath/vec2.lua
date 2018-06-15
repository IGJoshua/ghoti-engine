ffi.cdef[[

kmVec2* kmVec2Fill(kmVec2* pOut, kmScalar x, kmScalar y);

/** Returns the length of the vector*/
kmScalar kmVec2Length(const kmVec2* pIn);

/** Returns the square of the length of the vector*/
kmScalar kmVec2LengthSq(const kmVec2* pIn);

/** Returns the vector passed in set to unit length*/
kmVec2* kmVec2Normalize(kmVec2* pOut, const kmVec2* pIn);
kmVec2* kmVec2Lerp(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2,
                   kmScalar t);

/** Adds 2 vectors and returns the result*/
kmVec2* kmVec2Add(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2);

/** Returns the Dot product which is the cosine of the angle between
 * the two vectors multiplied by their lengths */
kmScalar kmVec2Dot(const kmVec2* pV1, const kmVec2* pV2);
kmScalar kmVec2Cross(const kmVec2* pV1, const kmVec2* pV2);

/** Subtracts 2 vectors and returns the result*/
kmVec2* kmVec2Subtract(kmVec2* pOut, const kmVec2* pV1, const kmVec2* pV2);

/** Component-wise multiplication */
kmVec2* kmVec2Mul( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 );

/** Component-wise division*/
kmVec2* kmVec2Div( kmVec2* pOut,const kmVec2* pV1, const kmVec2* pV2 );

/** Transform the Vector */
kmVec2* kmVec2Transform(kmVec2* pOut, const kmVec2* pV1,
                        const kmMat3* pM);

 /**Transforms a 2D vector by a given matrix, projecting the result
  * back into w = 1.*/
kmVec2* kmVec2TransformCoord(kmVec2* pOut, const kmVec2* pV,
                             const kmMat3* pM);

/** Scales a vector to length s*/
kmVec2* kmVec2Scale(kmVec2* pOut, const kmVec2* pIn, const kmScalar s);

/** Returns 1 if both vectors are equal*/
kmBool kmVec2AreEqual(const kmVec2* p1, const kmVec2* p2);

/**
 * Assigns pIn to pOut. Returns pOut. If pIn and pOut are the same
 * then nothing happens but pOut is still returned
 */
kmVec2* kmVec2Assign(kmVec2* pOut, const kmVec2* pIn);

/**
 * Rotates the point anticlockwise around a center by an amount of
 * degrees.
 */
kmVec2* kmVec2RotateBy(kmVec2* pOut, const kmVec2* pIn, const kmScalar degrees,
                       const kmVec2* center);

/**
 * 	Returns the angle in degrees between the two vectors
 */
kmScalar kmVec2DegreesBetween(const kmVec2* v1, const kmVec2* v2);

/**
 * Returns the distance between the two points
 */
kmScalar kmVec2DistanceBetween(const kmVec2* v1, const kmVec2* v2);

/**
 * Returns the point mid-way between two others
 */
kmVec2* kmVec2MidPointBetween(kmVec2* pOut, const kmVec2* v1, const kmVec2* v2);

/** Reflects a vector about a given surface normal. The surface normal
 * is assumed to be of unit length. */
kmVec2* kmVec2Reflect(kmVec2* pOut, const kmVec2* pIn, const kmVec2* normal);

void kmVec2Swap(kmVec2* pA, kmVec2* pB);

]]
