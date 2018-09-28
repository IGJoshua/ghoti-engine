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
]]

local component = engine.components:register("rigid_body", "RigidBodyComponent")

local C = engine.C
local kazmath = engine.kazmath

function component:addAcceleration(acceleration, position)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, acceleration, self.mass)

  local centerOfMass = ffi.new("kmVec3")
  position = position or centerOfMass

  C.addForce(self, force[0], position)
end

function component:addForce(force, position)
  local centerOfMass = ffi.new("kmVec3")
  position = position or centerOfMass

  C.addForce(self, force, position)
end

function component:addTorque(torque)
  C.addTorque(self, torque)
end

function component:setAcceleration(acceleration)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, acceleration, self.mass)

  C.setForce(self, force[0])
end

function component:setForce(force)
  C.setForce(self, force)
end

function component:setTorque(torque)
  C.setTorque(self, torque)
end

function component:zeroForce()
  local force = ffi.new("kmVec3")
  C.setForce(self, force)
end

function component:zeroTorque()
  local torque = ffi.new("kmVec3")
  C.setTorque(self, torque)
end

function component:getAcceleration()
  local acceleration = ffi.new("kmVec3[1]")
  acceleration[0] = C.getForce(self)
  kazmath.kmVec3Scale(acceleration, acceleration[0], 1.0 / self.mass)
  return acceleration[0]
end

function component:getForce()
  return C.getForce(self)
end

function component:getTorque()
  return C.getTorque(self)
end