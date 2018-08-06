ffi.cdef[[
typedef struct orbit_component_t
{
  kmVec3 origin;
  float speed;
  float radius;
  float time;
} OrbitComponent;
]]

-- io.write("Defined Orbit component for FFI\n")

local component = engine.components:register("orbit", "OrbitComponent")

-- io.write("Registered Orbit component\n")
