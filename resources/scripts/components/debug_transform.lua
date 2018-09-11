ffi.cdef[[
typedef struct debug_transform_component_t
{
	bool recursive;
	real32 lineWidth;
	real32 scale;
	kmVec3 xAxisColor;
	kmVec3 yAxisColor;
	kmVec3 zAxisColor;
} DebugTransformComponent;
]]

local component = engine.components:register("debug_transform", "DebugTransformComponent")