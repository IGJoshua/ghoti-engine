ffi.cdef[[
typedef struct orbit_component_t
{
	kmVec3 origin;
	real32 speed;
	real32 radius;
	real32 time;
} OrbitComponent;
]]

local component = engine.components:register("orbit", "OrbitComponent")