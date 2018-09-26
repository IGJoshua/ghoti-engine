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

local C = engine.C

function component:swap(name)
  C.freeImage(self.name)
  self.name = name
  C.loadImage(name)
end