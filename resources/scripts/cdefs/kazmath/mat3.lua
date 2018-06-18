ffi.cdef[[

kmMat3* kmMat3Fill(kmMat3* pOut, const kmScalar* pMat);
kmMat3* kmMat3Adjugate(kmMat3* pOut, const kmMat3* pIn);

/** Sets pOut to an identity matrix returns pOut*/
kmMat3* kmMat3Identity(kmMat3* pOut);
kmMat3* kmMat3Inverse(kmMat3* pOut, const kmMat3* pM);

/** Returns true if pIn is an identity matrix */
kmBool kmMat3IsIdentity(const kmMat3* pIn);

/** Sets pOut to the transpose of pIn, returns pOut */
kmMat3* kmMat3Transpose(kmMat3* pOut, const kmMat3* pIn);
kmScalar kmMat3Determinant(const kmMat3* pIn);

/** Returns true if the 2 matrices are equal (approximately) */
kmBool kmMat3AreEqual(const kmMat3* pMat1, const kmMat3* pMat2);

/** Assigns the value of pIn to pOut */
kmMat3* kmMat3AssignMat3(kmMat3* pOut, const kmMat3* pIn);

/* Multiplies pM1 with pM2, stores the result in pOut, returns pOut */
kmMat3* kmMat3MultiplyMat3(kmMat3* pOut, const kmMat3* lhs, const kmMat3* rhs);
kmMat3* kmMat3MultiplyScalar(kmMat3* pOut, const kmMat3* lhs,
                             const kmScalar rhs);

/**
 * Builds an X-axis rotation matrix and stores it in pOut, returns pOut
 */
kmMat3* kmMat3FromRotationX(kmMat3* pOut, const kmScalar radians);

/**
 * Builds a rotation matrix using the rotation around the Y-axis
 * The result is stored in pOut, pOut is returned.
 */
kmMat3* kmMat3FromRotationY(kmMat3* pOut, const kmScalar radians);

/**
 * Builds a rotation matrix around the Z-axis. The resulting
 * matrix is stored in pOut. pOut is returned.
 */
kmMat3* kmMat3FromRotationZ(kmMat3* pOut, const kmScalar radians);
kmMat3* kmMat3FromRotationXInDegrees(kmMat3* pOut, const kmScalar degrees);
kmMat3* kmMat3FromRotationYInDegrees(kmMat3* pOut, const kmScalar degrees);
kmMat3* kmMat3FromRotationZInDegrees(kmMat3* pOut, const kmScalar degrees);
kmMat3* kmMat3FromRotationQuaternion(kmMat3* pOut,
                                     const kmQuaternion* quaternion);
kmMat3* kmMat3FromRotationLookAt(kmMat3* pOut, const kmVec3* pEye,
                                 const kmVec3* pCentre,
                                 const kmVec3* pUp);

/** Builds a scaling matrix */
kmMat3* kmMat3FromScaling(kmMat3* pOut, const kmScalar x, const kmScalar y);
kmMat3* kmMat3FromTranslation(kmMat3* pOut, const kmScalar x, const kmScalar y);
kmMat3* kmMat3FromRotationAxisAngle(kmMat3* pOut, const kmVec3* axis, const kmScalar radians);
kmMat3* kmMat3FromRotationAxisAngleInDegrees(kmMat3* pOut, const kmVec3* axis, const kmScalar degrees);

void kmMat3ExtractRotationAxisAngle(const kmMat3* self, kmVec3* axis, kmScalar* radians);
void kmMat3ExtractRotationAxisAngleInDegrees(const kmMat3* self, kmVec3* axis, kmScalar* degrees);

kmVec3* kmMat3ExtractUpVec3(const kmMat3* self, kmVec3* pOut);
kmVec3* kmMat3ExtractRightVec3(const kmMat3* self, kmVec3* pOut);
kmVec3* kmMat3ExtractForwardVec3(const kmMat3* self, kmVec3* pOut);

]]
