ffi.cdef[[
typedef struct aabb_component_t
{
  kmVec3 bounds;
} BoxComponent;
]]

local component = engine.components:register("box", "BoxComponent")
