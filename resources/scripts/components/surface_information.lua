ffi.cdef[[
typedef struct surface_information_component_t
{
  bool finiteFriction;
  real32 friction;
  real32 bounciness;
  real32 bounceVelocity;
  bool disableRolling;
  real32 rollingFriction;
  real32 spinningFriction;
} SurfaceInformationComponent;
]]

local component = engine.components:register("surface_information", "SurfaceInformationComponent")
