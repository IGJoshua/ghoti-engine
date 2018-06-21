### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmRay3

# kmRay3
A structure that represents a three dimensional ray
```c
typedef struct kmRay3 {
    kmVec3 start;
    kmVec3 dir;
} kmRay3;
```

## kmRay3* kmRay3Fill
Fills a ray with the elements listed
```c
kmRay3* kmRay3Fill(kmRay3* ray, kmScalar px, kmScalar py, kmScalar pz, kmScalar vx, kmScalar vy, kmScalar vz);
```
## kmRay3* kmRay3FromPointAndDirection
Builds a ray from a start point and a direction
```c
kmRay3* kmRay3FromPointAndDirection(kmRay3* ray, const kmVec3* point, const kmVec3* direction);
```
## kmBool kmRay3IntersectPlane
Returns `KM_TRUE` if the ray intersects with the plane, otherwise returns `KM_FALSE`
```c
kmBool kmRay3IntersectPlane(kmVec3* pOut, const kmRay3* ray, const struct kmPlane* plane);
```
## kmBool kmRay3IntersectTriangle
Returns `KM_TRUE` if the ray intersects with the plane, otherwise returns `KM_FALSE`.
Triangle is defined by a the three pts. It outputs the vector
multiply factor that gives the intersection point and the normal is would create by bouncing. 
```c
kmBool kmRay3IntersectTriangle(const kmRay3* ray, const kmVec3* v0, const kmVec3* v1, const kmVec3* v2, kmVec3* intersection, kmVec3* normal, kmScalar* distance);
```
## kmBool kmRay3IntersectAABB3
Returns `KM_TRUE` if the ray intersects with the AABB, otherwise returns `KM_FALSE`. It outputs the vector
multiply factor that gives the intersection point
```c
kmBool kmRay3IntersectAABB3(const kmRay3* ray, const struct kmAABB3* aabb, kmVec3* intersection, kmScalar* distance);
```
