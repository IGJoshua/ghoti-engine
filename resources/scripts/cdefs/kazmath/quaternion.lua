ffi.cdef[[


int32 kmQuaternionAreEqual(const kmQuaternion* p1, const kmQuaternion* p2);
kmQuaternion* kmQuaternionFill(kmQuaternion* pOut, kmScalar x, kmScalar y,
                               kmScalar z, kmScalar w);

/** Returns the dot product of the 2 quaternions */
kmScalar kmQuaternionDot(const kmQuaternion* q1, const kmQuaternion* q2);

/** Returns the exponential of the quaternion (not implemented) */
kmQuaternion* kmQuaternionExp(kmQuaternion* pOut, const kmQuaternion* pIn);

/** Makes the passed quaternion an identity quaternion */
kmQuaternion* kmQuaternionIdentity(kmQuaternion* pOut);

/** Returns the inverse of the passed Quaternion */
kmQuaternion* kmQuaternionInverse(kmQuaternion* pOut, const kmQuaternion* pIn);

/** Returns true if the quaternion is an identity quaternion */
int32 kmQuaternionIsIdentity(const kmQuaternion* pIn);

/** Returns the length of the quaternion */
kmScalar kmQuaternionLength(const kmQuaternion* pIn);

/** Returns the length of the quaternion squared (prevents a sqrt) */
kmScalar kmQuaternionLengthSq(const kmQuaternion* pIn);

/** Returns the natural logarithm */
kmQuaternion* kmQuaternionLn(kmQuaternion* pOut, const kmQuaternion* pIn);

/** Multiplies 2 quaternions together */
kmQuaternion* kmQuaternionMultiply(kmQuaternion* pOut, const kmQuaternion* q1,
                                   const kmQuaternion* q2);

/** Normalizes a quaternion */
kmQuaternion* kmQuaternionNormalize(kmQuaternion* pOut,
                                    const kmQuaternion* pIn);

/** Rotates a quaternion around an axis */
kmQuaternion* kmQuaternionRotationAxisAngle(kmQuaternion* pOut,
                                            const kmVec3* pV,
                                            kmScalar angle);

/** Creates a quaternion from a rotation matrix */
kmQuaternion* kmQuaternionRotationMatrix(kmQuaternion* pOut,
                                         const kmMat3* pIn);

/** Create a quaternion from yaw, pitch and roll */
kmQuaternion* kmQuaternionRotationPitchYawRoll(kmQuaternion* pOut,
                                               kmScalar pitch,
                                               kmScalar yaw, kmScalar roll);

/** Interpolate between 2 quaternions */
kmQuaternion* kmQuaternionSlerp(kmQuaternion* pOut, const kmQuaternion* q1,
                                const kmQuaternion* q2, kmScalar t);

/** Get the axis and angle of rotation from a quaternion */
void kmQuaternionToAxisAngle(const kmQuaternion* pIn, kmVec3* pVector,
                             kmScalar* pAngle);

/** Scale a quaternion */
kmQuaternion* kmQuaternionScale(kmQuaternion* pOut, const kmQuaternion* pIn,
                                kmScalar s);
kmQuaternion* kmQuaternionAssign(kmQuaternion* pOut, const kmQuaternion* pIn);
kmQuaternion* kmQuaternionAdd(kmQuaternion* pOut, const kmQuaternion* pQ1,
                              const kmQuaternion* pQ2);
kmQuaternion* kmQuaternionSubtract(kmQuaternion* pOut, const kmQuaternion* pQ1,
                                   const kmQuaternion* pQ2);


/*
 *  Gets the shortest arc quaternion to rotate this vector to the
 *  destination vector.

 * If you call this with a dest vector that is close to the inverse of
 * this vector, we will rotate 180 degrees around the 'fallbackAxis'
 * (if specified, or a generated axis if not) since in this case ANY
 * axis of rotation is valid. */
kmQuaternion* kmQuaternionRotationBetweenVec3(kmQuaternion* pOut,
                                              const kmVec3* vec1,
                                              const kmVec3* vec2,
                                              const kmVec3* fallback);

kmVec3* kmQuaternionMultiplyVec3(kmVec3* pOut,
                                        const kmQuaternion* q,
                                        const kmVec3* v);

kmVec3* kmQuaternionGetUpVec3(kmVec3* pOut, const kmQuaternion* pIn);
kmVec3* kmQuaternionGetRightVec3(kmVec3* pOut, const kmQuaternion* pIn);
kmVec3* kmQuaternionGetForwardVec3RH(kmVec3* pOut, const kmQuaternion* pIn);
kmVec3* kmQuaternionGetForwardVec3LH(kmVec3* pOut, const kmQuaternion* pIn);

kmScalar kmQuaternionGetPitch(const kmQuaternion* q);
kmScalar kmQuaternionGetYaw(const kmQuaternion* q);
kmScalar kmQuaternionGetRoll(const kmQuaternion* q);

kmQuaternion* kmQuaternionLookRotation(kmQuaternion* pOut,
                                       const kmVec3* direction,
                                       const kmVec3* up);

/* Given a quaternion, and an axis. This extracts the rotation around
 * the axis into pOut as another quaternion. Uses the swing-twist
 * decomposition. */
kmQuaternion* kmQuaternionExtractRotationAroundAxis(const kmQuaternion* pIn,
                                                    const kmVec3* axis,
                                                    kmQuaternion* pOut);

/*
 * Returns a Quaternion representing the angle between two vectors
 */
kmQuaternion* kmQuaternionBetweenVec3(kmQuaternion* pOut, const kmVec3* v1,
                                      const kmVec3* v2);

]]
