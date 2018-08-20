ffi.cdef[[
typedef struct wireframe_component_t
{
	bool visible;
	real32 lineWidth;
	bool customColor;
	kmVec3 color;
} WireframeComponent;
]]

io.write("Defined Wireframe component for FFI\n")

local component = engine.components:register("wireframe", "WireframeComponent")

io.write("Registered Wireframe component\n")
