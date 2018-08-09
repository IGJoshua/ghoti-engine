ffi.cdef[[
typedef enum moment_of_inertia_e
{
  MOMENT_OF_INERTIA_USER = -1,
  MOMENT_OF_INERTIA_DEFAULT = 0,
  MOMENT_OF_INERTIA_SPHERE = 1,
  MOMENT_OF_INERTIA_CUBE,
  MOMENT_OF_INERTIA_CAPSULE
} MomentOfInertia;

typedef struct rigid_body_component_t
{
  int64 bodyID;
  int64 spaceID;
  Bool enabled;
  Bool dynamic;
  Bool gravity;
  real32 mass;
  kmVec3 centerOfMass;
  kmVec3 velocity;
  kmVec3 angularVel;
  Bool defaultDamping;
  real32 linearDamping;
  real32 angularDamping;
  real32 linearDampingThreshold;
  real32 angularDampingThreshold;
  real32 maxAngularSpeed;
  MomentOfInertia inertiaType;
  real32 moiParams[6];
} RigidBodyComponent;
]]

local component = engine.components:register("rigid_body", "RigidBodyComponent")
