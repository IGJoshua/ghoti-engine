ffi.cdef[[
typedef struct hit_information_component_t
{
  int32 age;
  UUID volume1;
  UUID volume2;
  UUID object1;
  UUID object2;
  kmVec3 contactNormal;
  kmVec3 position;
  real32 depth;
} HitInformationComponent;
]]

local component = engine.components:register("hit_information", "HitInformationComponent")
