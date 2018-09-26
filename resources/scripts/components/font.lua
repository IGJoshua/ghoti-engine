ffi.cdef[[
typedef struct font_component_t
{
	char name[64];
	real32 size;
	kmVec4 color;
} FontComponent;
]]

local component = engine.components:register("font", "FontComponent")

local C = engine.C

function component:swap(name, size)
  self.name = name
  self.size = size
  C.loadFont(name, size)
end