ffi.cdef[[

/**
    Initializes the AABB around a central point. If centre is NULL
    then the origin is used. Returns pBox.
*/
kmAABB2* kmAABB2Initialize(kmAABB2* pBox, const kmVec2* centre,
                           const kmScalar width, const kmScalar height,
                           const kmScalar depth);

/**
 *  Makes sure that min corresponds to the minimum values and max to
 *  the maximum
 */
kmAABB2* kmAABB2Sanitize(kmAABB2* pOut, const kmAABB2* pIn );

/**
 * Returns KM_TRUE if point is in the specified AABB, returns KM_FALSE
 * otherwise.
 */
int32 kmAABB2ContainsPoint(const kmAABB2* pBox, const kmVec2* pPoint);

/**
 * Assigns pIn to pOut, returns pOut.
 */
kmAABB2* kmAABB2Assign(kmAABB2* pOut, const kmAABB2* pIn);

/**
 * Scales pIn by s, stores the resulting AABB in pOut. Returns pOut.
 * It modifies both points, so position of the box will be
 * changed. Use kmAABB2ScaleWithPivot to specify the origin of the
 * scale.
 */
kmAABB2* kmAABB2Translate(kmAABB2* pOut, const kmAABB2* pIn,
                          const kmVec2 *translation );

kmAABB2* kmAABB2Scale(kmAABB2* pOut, const kmAABB2* pIn, kmScalar s);

/**
 * Scales pIn by s, using pivot as the origin for the scale.
 */
kmAABB2* kmAABB2ScaleWithPivot( kmAABB2* pOut, const kmAABB2* pIn,
                                const kmVec2 *pivot, kmScalar s );

kmEnum kmAABB2ContainsAABB(const kmAABB2* container, const kmAABB2* to_check);
kmScalar kmAABB2DiameterX(const kmAABB2* aabb);
kmScalar kmAABB2DiameterY(const kmAABB2* aabb);
kmVec2* kmAABB2Centre(const kmAABB2* aabb, kmVec2* pOut);

/**
 * @brief kmAABB2ExpandToContain
 * @param pOut - The resulting AABB
 * @param pIn - The original AABB
 * @param other - Another AABB that you want pIn expanded to contain
 * @return
 */
kmAABB2* kmAABB2ExpandToContain(kmAABB2* pOut, const kmAABB2* pIn,
                                const kmAABB2* other);

]]
