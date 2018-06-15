ffi.cdef[[

/**
    Initializes the AABB around a central point. If centre is NULL
    then the origin is used. Returns pBox.
*/
kmAABB3* kmAABB3Initialize(kmAABB3* pBox, const kmVec3* centre,
                           const kmScalar width, const kmScalar height,
                           const kmScalar depth);

/**
 * Returns KM_TRUE if point is in the specified AABB, returns KM_FALSE
 * otherwise.
 */
int32 kmAABB3ContainsPoint(const kmAABB3* pBox, const kmVec3* pPoint);

/**
 * Assigns pIn to pOut, returns pOut.
 */
kmAABB3* kmAABB3Assign(kmAABB3* pOut, const kmAABB3* pIn);

/**
 * Scales pIn by s, stores the resulting AABB in pOut. Returns pOut
 */
kmAABB3* kmAABB3Scale(kmAABB3* pOut, const kmAABB3* pIn, kmScalar s);
kmBool kmAABB3IntersectsTriangle(kmAABB3* box, const kmVec3* p1,
                                 const kmVec3* p2, const kmVec3* p3);
kmBool kmAABB3IntersectsAABB(const kmAABB3* box, const kmAABB3* other);
kmEnum kmAABB3ContainsAABB(const kmAABB3* container, const kmAABB3* to_check);
kmScalar kmAABB3DiameterX(const kmAABB3* aabb);
kmScalar kmAABB3DiameterY(const kmAABB3* aabb);
kmScalar kmAABB3DiameterZ(const kmAABB3* aabb);
kmVec3* kmAABB3Centre(const kmAABB3* aabb, kmVec3* pOut);

/**
 * @brief kmAABB3ExpandToContain
 * @param pOut - The resulting AABB
 * @param pIn - The original AABB
 * @param other - Another AABB that you want pIn expanded to contain
 * @return
 */
kmAABB3* kmAABB3ExpandToContain(kmAABB3* pOut, const kmAABB3* pIn, const kmAABB3* other);



]]
