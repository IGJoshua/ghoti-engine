ffi.cdef[[
typedef struct font_component_t
{
	char name[64];
	real32 size;
	bool autoScaling;
	kmVec4 color;
} FontComponent;
]]

local component = engine.components:register("font", "FontComponent")

local C = engine.C

function component:swap(name, size, autoScaling)
  self.name = name
  self.size = size
  self.autoScaling = autoScaling

  C.loadFont(name, size, autoScaling)
end