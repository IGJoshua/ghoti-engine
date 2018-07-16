### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmPlane

# kmPlane
A structure that represents a plane
```c
typedef struct kmPlane {
	kmScalar 	a, b, c, d;
} kmPlane;
```

## KM_POINT_CLASSIFICATION
An enumeration used to describe the location of an object in relation to a `kmPlane`
```c
typedef enum KM_POINT_CLASSIFICATION {
    POINT_BEHIND_PLANE = -1,
    POINT_ON_PLANE = 0,
    POINT_INFRONT_OF_PLANE = 1
} KM_POINT_CLASSIFICATION;
```

## kmPlane* kmPlaneFill
Fills the plane with the values entered
```C
kmPlane* kmPlaneFill(kmPlane* plane, kmScalar a, kmScalar b, kmScalar c,
                     kmScalar d);
```
## kmScalar kmPlaneDot / kmScalar kmPlaneDotCoord / kmScalar kmPlaneDotNormal
Returns the dot product in various circumstances
```c
kmScalar kmPlaneDot(const kmPlane* pP, const struct kmVec4* pV);
kmScalar kmPlaneDotCoord(const kmPlane* pP, const struct kmVec3* pV);
kmScalar kmPlaneDotNormal(const kmPlane* pP, const struct kmVec3* pV);
```
## kmPlane* kmPlaneFromNormalAndDistance
Builds a plane from its normal and distance from the origin
```c
kmPlane* kmPlaneFromNormalAndDistance(kmPlane* plane,
                                      const struct kmVec3* normal,
                                      const kmScalar dist);
```
## kmPlane* kmPlaneFromPointAndNormal
Builds a plane from its normal and distance from the origin
```c
kmPlane* kmPlaneFromPointAndNormal(kmPlane* pOut, const struct kmVec3* pPoint,
                                   const struct kmVec3* pNormal);
```
## kmPlane* kmPlaneFromPoints
Creates a plane from 3 points. The result is stored in `pOut`.
`pOut` is returned.
```c
kmPlane* kmPlaneFromPoints(kmPlane* pOut, const struct kmVec3* p1,
                           const struct kmVec3* p2, const struct kmVec3* p3);
```
## struct kmVec3* kmPlaneIntersectLine
Returns where line `pV1`-`pV2` intersects plane `pP`. Returns NULL if the line
is not intersecting plane `pP`
```c
struct kmVec3* kmPlaneIntersectLine(struct kmVec3* pOut, const kmPlane* pP,
                                    const struct kmVec3* pV1,
                                    const struct kmVec3* pV2);
```
## kmPlane* kmPlaneNormalize
Returns the normailzed version of `pP`
```c
kmPlane* kmPlaneNormalize(kmPlane* pOut, const kmPlane* pP);
```
## kmPlane* kmPlaneScale
Returns a scaled version of `pP` by a factor of `s`
```c
kmPlane* kmPlaneScale(kmPlane* pOut, const kmPlane* pP, kmScalar s);
```
## KM_POINT_CLASSIFICATION kmPlaneClassifyPoint
Returns POINT_INFRONT_OF_PLANE if pP is in front of pIn. Returns
POINT_BEHIND_PLANE if it is behind. Returns POINT_ON_PLANE otherwise
```c
KM_POINT_CLASSIFICATION kmPlaneClassifyPoint(const kmPlane* pIn,
                                             const struct kmVec3* pP);
```
## kmPlane* kmPlaneExtractFromMat4
Extract a plane from the input 4x4 transformation.
Stores the result in `pOut`, returns `pOut`
```c
kmPlane* kmPlaneExtractFromMat4(kmPlane* pOut, const struct kmMat4* pIn,
                                kmInt row);
```                            
## kmVec3* kmPlaneGetIntersection
Returns the point where all three plains intersect
```c
struct kmVec3* kmPlaneGetIntersection(struct kmVec3* pOut, const kmPlane* p1,
                                      const kmPlane* p2, const kmPlane* p3);
```
