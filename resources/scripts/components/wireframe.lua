ffi.cdef[[
typedef struct wireframe_component_t
{
	bool visible;
	real32 lineWidth;
	kmVec3 scale;
	kmVec3 color;
} WireframeComponent;
]]

local component = engine.components:register("wireframe", "WireframeComponent")
