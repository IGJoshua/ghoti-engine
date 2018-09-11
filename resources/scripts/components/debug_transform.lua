ffi.cdef[[
typedef struct debug_transform_component_t
{
	bool recursive;
	real32 lineWidth;
	real32 scale;
	kmVec3 xAxisColor;
	kmVec3 xAxisEndpointColor;
	kmVec3 yAxisColor;
	kmVec3 yAxisEndpointColor;
	kmVec3 zAxisColor;
	kmVec3 zAxisEndpointColor;
} DebugTransformComponent;
]]

local component = engine.components:register("debug_transform", "DebugTransformComponent")