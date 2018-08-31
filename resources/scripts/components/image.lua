ffi.cdef[[
typedef struct image_component_t
{
	char name[64];
	kmVec4 color;
	kmVec2 position;
	kmVec2 scale;
	int32 pivot;
} ImageComponent;
]]

local component = engine.components:register("image", "ImageComponent")
