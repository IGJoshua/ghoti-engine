ffi.cdef[[
typedef struct hit_list_component_t
{
  UUID nextHit;
  UUID hit;
} HitListComponent;
]]

local component = engine.components:register("hit_list", "HitListComponent")
