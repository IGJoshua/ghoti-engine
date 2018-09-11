ffi.cdef[[
typedef struct debug_primitive_component_t
{
	bool visible;
} DebugPrimitiveComponent;
]]

local component = engine.components:register("debug_primitive", "DebugPrimitiveComponent")