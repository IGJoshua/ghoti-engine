ffi.cdef[[
typedef struct model_component_t
{
	char name[1024];
	bool visible;
	bool wireframe;
	real32 wireframeLineWidth;
	bool customWireframeColor;
	kmVec3 wireframeColor;
} ModelComponent;
]]

io.write("Defined Model component for FFI\n")

local component = engine.components:register("model", "ModelComponent")

io.write("Registered Model component\n")
