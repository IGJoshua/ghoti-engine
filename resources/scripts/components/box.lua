ffi.cdef[[
typedef struct obb_component_t
{
  kmVec3 bounds;
} BoxComponent;
]]

local component = engine.components:register("box", "BoxComponent")
