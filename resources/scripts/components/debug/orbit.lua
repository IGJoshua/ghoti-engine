ffi.cdef[[
typedef struct orbit_component_t
{
	kmVec3 origin;
	real32 speed;
	real32 radius;
	real32 time;
} OrbitComponent;
]]

-- io.write("Defined Orbit component for FFI\n")

local component = engine.components:register("orbit", "OrbitComponent")

-- io.write("Registered Orbit component\n")
