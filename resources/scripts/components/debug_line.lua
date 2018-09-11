ffi.cdef[[
typedef struct debug_line_component_t
{
	UUID endpoint;
	real32 lineWidth;
	kmVec3 color;
	kmVec3 endpointColor;
} DebugLineComponent;
]]

local component = engine.components:register("debug_line", "DebugLineComponent")