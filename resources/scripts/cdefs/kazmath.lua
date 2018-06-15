ffi.cdef[[


typedef real32 kmScalar;
typedef real32 kmEpsilon;

typedef uint8 kmBool;
typedef uint8 kmUchar;

typedef uint32 kmEnum;
typedef uint32 kmUint;

typedef int32 kmInt;

kmScalar kmSQR(kmScalar s);

kmScalar kmDegreesToRadians(kmScalar degrees);

kmScalar kmRadiansToDegrees(kmScalar radians);

kmScalar kmMin(kmScalar lhs, kmScalar rhs);
kmScalar kmMax(kmScalar lhs, kmScalar rhs);
kmBool kmAlmostEqual(kmScalar lhs, kmScalar rhs);

kmScalar kmClamp(kmScalar x, kmScalar min, kmScalar max);
kmScalar kmLerp(kmScalar x, kmScalar y, kmScalar factor);

typedef struct kmVec2 { //extern consts
    kmScalar x;
    kmScalar y;
} kmVec2;

const kmVec2 KM_VEC2_POS_Y = { 0, 1 };
const kmVec2 KM_VEC2_NEG_Y = { 0, -1 };
const kmVec2 KM_VEC2_NEG_X = { -1, 0 };
const kmVec2 KM_VEC2_POS_X = { 1, 0 };
const kmVec2 KM_VEC2_ZERO = { 0, 0 };

typedef struct kmVec3 { //extern consts
    kmScalar x;
    kmScalar y;
    kmScalar z;
} kmVec3;

const kmVec3 KM_VEC3_POS_Z = { 0, 0, 1 };
const kmVec3 KM_VEC3_NEG_Z = { 0, 0, -1 };
const kmVec3 KM_VEC3_POS_Y = { 0, 1, 0 };
const kmVec3 KM_VEC3_NEG_Y = { 0, -1, 0 };
const kmVec3 KM_VEC3_NEG_X = { -1, 0, 0 };
const kmVec3 KM_VEC3_POS_X = { 1, 0, 0 };
const kmVec3 KM_VEC3_ZERO = { 0, 0, 0 };

typedef struct kmVec4 { //extern consts
	kmScalar x;
	kmScalar y;
	kmScalar z;
	kmScalar w;
} kmVec4;

typedef struct kmAABB2 {
    kmVec2 min; /** The max corner of the box */
    kmVec2 max; /** The min corner of the box */
} kmAABB2;

typedef struct kmAABB3 {
    kmVec3 min; /** The max corner of the box */
    kmVec3 max; /** The min corner of the box */
} kmAABB3;

typedef struct kmMat3{
	kmScalar mat[9];
} kmMat3;

/*
A 4x4 matrix

      | 0   4   8  12 |
mat = | 1   5   9  13 |
      | 2   6  10  14 |
      | 3   7  11  15 |
*/
typedef struct kmMat4 {
	kmScalar mat[16];
} kmMat4;

typedef struct kmPlane {
	kmScalar 	a, b, c, d;
} kmPlane;

typedef struct kmRay2 {
    kmVec2 start;
    kmVec2 dir;
} kmRay2;

typedef struct kmRay3 {
    kmVec3 start;
    kmVec3 dir;
} kmRay3;

typedef struct kmQuaternion {
    kmScalar x;
    kmScalar y;
    kmScalar z;
    kmScalar w;
} kmQuaternion;

]]

local kazAabb2      = require("aabb2")
local kazAabb3      = require("aabb3")
local kazMat3       = require("mat3")
local kazMat4       = require("mat4")
local kazPlane      = require("plane")
local kazQuat       = require("quaternion")
local kazRay2       = require{"ray2")
local kazRay3       = require{"ray3")
local kazVec2       = require("vec2")
local kazVec3       = require("vec3")
local kazVec4       = require("vec4")
