###[Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> Plane

#kmPlane
A structure that represents a plane with 4 points
```
typedef struct kmPlane {
	kmScalar 	a, b, c, d;
} kmPlane;
```
##KM_POINT_CLASSIFICATION
A enumeration used to represent the location of an object in relation to the plane
typedef enum KM_POINT_CLASSIFICATION {
    POINT_BEHIND_PLANE = -1,
    POINT_ON_PLANE = 0,
    POINT_INFRONT_OF_PLANE = 1
} KM_POINT_CLASSIFICATION;

kmPlane* kmPlaneFill(kmPlane* plane, kmScalar a, kmScalar b, kmScalar c,
                     kmScalar d);
kmScalar kmPlaneDot(const kmPlane* pP, const struct kmVec4* pV);
kmScalar kmPlaneDotCoord(const kmPlane* pP, const struct kmVec3* pV);
kmScalar kmPlaneDotNormal(const kmPlane* pP, const struct kmVec3* pV);
kmPlane* kmPlaneFromNormalAndDistance(kmPlane* plane,
                                      const struct kmVec3* normal,
                                      const kmScalar dist);
kmPlane* kmPlaneFromPointAndNormal(kmPlane* pOut, const struct kmVec3* pPoint,
                                   const struct kmVec3* pNormal);

/**
 * Creates a plane from 3 points. The result is stored in pOut.
 * pOut is returned.
 */
kmPlane* kmPlaneFromPoints(kmPlane* pOut, const struct kmVec3* p1,
                           const struct kmVec3* p2, const struct kmVec3* p3);
struct kmVec3* kmPlaneIntersectLine(struct kmVec3* pOut, const kmPlane* pP,
                                    const struct kmVec3* pV1,
                                    const struct kmVec3* pV2);
kmPlane* kmPlaneNormalize(kmPlane* pOut, const kmPlane* pP);
kmPlane* kmPlaneScale(kmPlane* pOut, const kmPlane* pP, kmScalar s);

/**
 * Returns POINT_INFRONT_OF_PLANE if pP is in front of pIn. Returns
 * POINT_BEHIND_PLANE if it is behind. Returns POINT_ON_PLANE otherwise
 */
KM_POINT_CLASSIFICATION kmPlaneClassifyPoint(const kmPlane* pIn,
                                             const struct kmVec3* pP);

kmPlane* kmPlaneExtractFromMat4(kmPlane* pOut, const struct kmMat4* pIn,
                                kmInt row);
struct kmVec3* kmPlaneGetIntersection(struct kmVec3* pOut, const kmPlane* p1,
                                      const kmPlane* p2, const kmPlane* p3);
