### [Table of Contents](../Lua.md) -> [Kazmath](kazmath.md) -> kmAABB3


# kmAABB3

A structure that represents a three dimensional axis-aligned bounding box.
```c
typedef struct kmAABB3 {
    kmVec3 min; /** The max corner of the box */
    kmVec3 max; /** The min corner of the box */
} kmAABB3;
```

## kmAABB3* kmAABB3Initialize

Initializes the AABB around a central point. If `centre` is NULL then the origin is used. Returns `pBox`.

 ```c
kmAABB3* kmAABB3Initialize(kmAABB3* pBox, const kmVec3* centre,
                           const kmScalar width, const kmScalar height,
                           const kmScalar depth);
```

## int kmAABB3ContainsPoint

 Returns `KM_TRUE` if point is in the specified AABB, returns `KM_FALSE`
 otherwise.

```c
int kmAABB3ContainsPoint(const kmAABB3* pBox, const kmVec3* pPoint);
```
## kmAABB3* kmAABB3Assign
Assigns `pIn` to `pOut`, returns `pOut`.

```c
kmAABB3* kmAABB3Assign(kmAABB3* pOut, const kmAABB3* pIn);
```
## kmAABB3* kmAABB3Scale

 Scales `pIn` by `s`, stores the resulting AABB in `pOut`. Returns `pOut`

```c
kmAABB3* kmAABB3Scale(kmAABB3* pOut, const kmAABB3* pIn, kmScalar s);
```

## kmBool kmAABB3IntersectsTriangle

Returns `true` if the triangle made from `p1`, `p2`, and `p3` intersects `box`.
```c
kmBool kmAABB3IntersectsTriangle(kmAABB3* box, const kmVec3* p1,
                                 const kmVec3* p2, const kmVec3* p3);
```

## kmBool kmAABB3IntersectsTriangle

Returns `true` if the `other` AABB3 intersects `box`.
```c
kmBool kmAABB3IntersectsAABB(const kmAABB3* box, const kmAABB3* other);
```

## kmEnum kmAABB3ContainsAABB
Returns `KM_CONTAINS_NONE` if `to_check` is not contained by `container`  
Returns `KM_CONTAINS_PARTIAL` if `to_check` is partially contained by `container`  
Returns `KM_CONTAINS_ALL` if `to_check` is completely contained by `container`  
```c
kmEnum kmAABB3ContainsAABB(const kmAABB3* container, const kmAABB3* to_check);
```
## kmScalar kmAABB3Diameter
Returns a `kmScalar` for the diameter of X, Y, or Z
```c
kmScalar kmAABB3DiameterX(const kmAABB3* aabb);
kmScalar kmAABB3DiameterY(const kmAABB3* aabb);
kmScalar kmAABB3DiameterZ(const kmAABB3* aabb);
```

## kmVec3* kmAABB3Centre
Returns the center point of `aabb`
```c
kmVec3* kmAABB3Centre(const kmAABB3* aabb, kmVec3* pOut);
```

## kmAABB3* kmAABB3ExpandToContain
Expands `pIn` to contain `other`. Returns `pOut`.
```c
kmAABB3* kmAABB3ExpandToContain(kmAABB3* pOut, const kmAABB3* pIn, const kmAABB3* other);
```
