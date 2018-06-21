### [Table of Contents](../Lua.md) -> Kazmath  
# Kazmath  
A C Math library targeted for use in games.
## Types  

### kmScalar
A 32 bit floating point value.

### kmEpsilon

### kmBool
An unsigned 8 bit integer value used for `true` or `false` statements

### kmUchar
An unsigned 8 bit character

### kmEnum
An unsigned 32 bit integer used for enumerations

### kmUint
An unsigned 32 bit integer

### kmInt
A signed 32 bit integer

## Helper Functions
```c
//Squares a scalar
kmScalar kmSQR(kmScalar s);

//Converts Degrees to Radians or vice versa
kmScalar kmDegreesToRadians(kmScalar degrees);
kmScalar kmRadiansToDegrees(kmScalar radians);

//Returns the min or max
kmScalar kmMin(kmScalar lhs, kmScalar rhs);
kmScalar kmMax(kmScalar lhs, kmScalar rhs);

//Checks if the two values are within Epsilon of each other
kmBool kmAlmostEqual(kmScalar lhs, kmScalar rhs);

//Clamps x within min and max 
kmScalar kmClamp(kmScalar x, kmScalar min, kmScalar max);

//Linearly interpolates
kmScalar kmLerp(kmScalar x, kmScalar y, kmScalar factor);
```
## Kazmath Structures  
#### [kmAABB2](aabb2.md)  
A structure that represents a two dimensional axis-aligned bounding box.  
#### [kmAABB3](aabb3.md)  
A structure that represents a three dimensional axis-aligned bounding box.  
#### [kmMat3](mat3.md)  
A structure that represents a 3x3 matrix  
#### [kmMat4](mat4.md)  
A structure that represents a 4x4 matrix  
#### [kmPlane](plane.md)  
A structure that represents a plane  
#### [kmQuaternion](quaternion.md)  
A structure that represents a quaternion  
#### [kmRay2](ray2.md)  
A structure that represents a two dimensional ray  
#### [kmRay3](ray3.md)  
A structure that represents a three dimensional ray  
#### [kmVec2](vec2.md)  
A structure that represents a vector with 2 elements  
#### [kmVec3](vec3.md)  
A structure that represents a vector with 3 elements  
#### [kmVec4](vec4.md)  
A structure that represents a vector with 4 elements  
