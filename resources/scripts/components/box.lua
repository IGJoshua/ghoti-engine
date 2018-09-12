ffi.cdef[[
typedef struct box_component_t
{
  kmVec3 bounds;
} BoxComponent;
]]

local component = engine.components:register("box", "BoxComponent")
