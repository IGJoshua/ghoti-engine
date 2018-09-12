ffi.cdef[[
typedef struct debug_collision_primitive_component_t
{
	bool visible;
	bool recursive;
	real32 lineWidth;
	kmVec3 boxColor;
	kmVec3 sphereColor;
	kmVec3 capsuleColor;
} DebugCollisionPrimitiveComponent;
]]

local component = engine.components:register("debug_collision_primitive", "DebugCollisionPrimitiveComponent")