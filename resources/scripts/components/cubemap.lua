ffi.cdef[[
typedef struct cubemap_component_t
{
	char name[64];
} CubemapComponent;
]]

local component = engine.components:register("cubemap", "CubemapComponent")

local C = engine.C

function component:swap(name)
  self.name = name
  C.loadCubemap(name)
end