### [Table of Contents](../../main.md) -> [Lua](../Lua.md) -> [Kazmath](kazmath.md) -> kmAABB2

# kmAABB2

A structure that represents a two dimensional axis-aligned bounding box.
```c
typedef struct kmAABB2 {
    kmVec2 min; /* The max corner of the box */
    kmVec2 max; /* The min corner of the box */
} kmAABB2;
```
## kmAABB2* kmAABB2Initialize

Initializes the AABB around a central point. If `centre` is NULL
then the origin is used. Returns `pBox`.

```c
kmAABB2* kmAABB2Initialize(kmAABB2* pBox, const kmVec2* centre,
                           const kmScalar width, const kmScalar height,
                           const kmScalar depth);`
```

## kmAABB2* kmAABB2Sanitize

Makes sure that min corresponds to the minimum values and max to
the maximum

```c
kmAABB2* kmAABB2Sanitize(kmAABB2* pOut, const kmAABB2* pIn );
```

## int32 kmAABB2ContainsPoint

Returns `KM_TRUE` if point is in the specified AABB, returns `KM_FALSE`
otherwise.

`int32 kmAABB2ContainsPoint(const kmAABB2* pBox, const kmVec2* pPoint);`

## kmAABB2* kmAABB2Assign

Assigns `pIn` to `pOut`, returns `pOut`.

```c
kmAABB2* kmAABB2Assign(kmAABB2* pOut, const kmAABB2* pIn);
```

## kmAABB2Translate
Returns a copy of `pIn` translated by `translation`.  

```c
kmAABB2* kmAABB2Translate(kmAABB2* pOut, const kmAABB2* pIn,
                          const kmVec2 *translation );
```
## kmAABB2* kmAABB2Scale

Scales `pIn` by `s`, stores the resulting AABB in `pOut`. Returns `pOut`.
It modifies both points, so position of the box will be
changed. Use `kmAABB2ScaleWithPivot` to specify the origin of the
scale.

```c
kmAABB2* kmAABB2Scale(kmAABB2* pOut, const kmAABB2* pIn, kmScalar s);
```

## kmAABB2* kmAABB2ScaleWithPivot

Scales `pIn` by `s`, using pivot as the origin for the scale.

```c
kmAABB2* kmAABB2ScaleWithPivot( kmAABB2* pOut, const kmAABB2* pIn,
                                const kmVec2 *pivot, kmScalar s );
```
## kmEnum kmAABB2ContainsAABB

Returns `KM_CONTAINS_NONE` if `to_check` is not contained by `container`  
Returns `KM_CONTAINS_PARTIAL` if `to_check` is partially contained by `container`  
Returns `KM_CONTAINS_ALL` if `to_check` is completely contained by `container`  
```c
kmEnum kmAABB2ContainsAABB(const kmAABB2* container, const kmAABB2* to_check);
```

## kmScalar kmAABB2DiameterX / kmScalar kmAABB2DiameterY
Returns a scalar of the X or Y diameter.
```c
kmScalar kmAABB2DiameterX(const kmAABB2* aabb);
kmScalar kmAABB2DiameterY(const kmAABB2* aabb);
```

## kmVec2* kmAABB2Centre
Returns the center point of an AABB.
```c
kmVec2* kmAABB2Centre(const kmAABB2* aabb, kmVec2* pOut);
```

## kmAABB2* kmAABB2ExpandToContain

Expands `pIn` to contain `other`. Returns `pOut`.

```c
kmAABB2* kmAABB2ExpandToContain(kmAABB2* pOut, const kmAABB2* pIn,
                                const kmAABB2* other);
```
