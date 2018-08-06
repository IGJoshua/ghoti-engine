ffi.cdef[[
typedef struct capsule_component_t
{
  real32 radius;
  real32 length;
} CapsuleComponent;
]]

local component = engine.components:register("capsule", "CapsuleComponent")
