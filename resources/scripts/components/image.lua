ffi.cdef[[
typedef struct image_component_t
{
	char name[64];
	kmVec4 color;
	kmVec2 position;
	int32 positionMode;
	kmVec2 scale;
	int32 pivot;
} ImageComponent;
]]

local component = engine.components:register("image", "ImageComponent")

local C = engine.C

function component:swap(name)
  self.name = name
  C.loadImage(name)
end