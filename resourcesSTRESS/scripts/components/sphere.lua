ffi.cdef[[
typedef struct sphere_component_t
{
  real32 radius;
} SphereComponent;
]]

local component = engine.components:register("sphere", "SphereComponent")
