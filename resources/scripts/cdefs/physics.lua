ffi.cdef[[

typedef void *dBodyID;
typedef void *dSpaceID;

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
  dBodyID bodyID;
  dSpaceID spaceID;
  bool isTrigger;
  bool enabled;
  bool dynamic;
  bool gravity;
  real32 mass;
  kmVec3 centerOfMass;
  kmVec3 velocity;
  kmVec3 angularVel;
  bool defaultDamping;
  real32 linearDamping;
  real32 angularDamping;
  real32 linearDampingThreshold;
  real32 angularDampingThreshold;
  real32 maxAngularSpeed;
  MomentOfInertia inertiaType;
  real32 moiParams[6];
} RigidBodyComponent;

void registerRigidBody(Scene *scene, UUID entity, RigidBodyComponent *body);
void destroyRigidBody(RigidBodyComponent *body);
]]
