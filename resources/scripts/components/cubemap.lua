ffi.cdef[[
typedef struct cubemap_component_t
{
	char name[64];
	bool swapFrontAndBack;
} CubemapComponent;
]]

local component = engine.components:register("cubemap", "CubemapComponent")

local C = engine.C

function component:swap(name, swapFrontAndBack)
  self.name = name
  C.loadCubemap(name, swapFrontAndBack)
end