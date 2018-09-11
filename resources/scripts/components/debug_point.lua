ffi.cdef[[
typedef struct debug_point_component_t
{
	real32 size;
	kmVec3 color;
} DebugPointComponent;
]]

local component = engine.components:register("debug_point", "DebugPointComponent")