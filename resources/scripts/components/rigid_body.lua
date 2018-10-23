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

function component:addVelocity(velocity)
  kazmath.kmVec3Add(self.velocity, self.velocity, velocity)
end

function component:addVelocityDirection(magnitude, direction)
  local velocity = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(velocity, direction, magnitude)

  self:addVelocity(velocity[0])
end

function component:addVelocityQuaternion(magnitude, quaternion)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:addVelocityDirection(magnitude, direction[0])
end

function component:addAngularVelocity(angularVelocity)
  kazmath.kmVec3Add(self.angularVel, self.angularVel, angularVelocity)
end

function component:addAngularVelocityAxis(magnitude, axis)
  local angularVelocity = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(angularVelocity, axis, magnitude)

  self:addAngularVelocity(angularVelocity[0])
end

function component:addAngularVelocityQuaternion(magnitude, quaternion)
  local axis = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(axis, quaternion)

  self:addAngularVelocityAxis(magnitude, axis[0])
end

function component:addAcceleration(acceleration, position)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, acceleration, self.mass)

  local centerOfMass = ffi.new("kmVec3")
  position = position or centerOfMass

  self:addForce(force[0], position)
end

function component:addAccelerationDirection(magnitude, direction, position)
  local acceleration = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(acceleration, direction, magnitude)

  self:addAcceleration(acceleration[0], position)
end

function component:addAccelerationQuaternion(magnitude, quaternion, position)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:addAccelerationDirection(magnitude, direction[0], position)
end

function component:addForce(force, position)
  local centerOfMass = ffi.new("kmVec3")
  position = position or centerOfMass

  C.addForce(self, force, position)
end

function component:addForceDirection(magnitude, direction, position)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, direction, magnitude)

  self:addForce(force[0], position)
end

function component:addForceQuaternion(magnitude, quaternion, position)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:addForceDirection(magnitude, direction[0], position)
end

function component:addTorque(torque)
  C.addTorque(self, torque)
end

function component:addTorqueAxis(magnitude, axis)
  local torque = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(torque, axis, magnitude)

  self:addTorque(torque[0])
end

function component:addTorqueQuaternion(magnitude, quaternion)
  local axis = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(axis, quaternion)

  self:addTorqueAxis(magnitude, axis[0])
end

function component:setVelocity(velocity)
  kazmath.kmVec3Assign(self.velocity, velocity)
end

function component:setVelocityDirection(magnitude, direction)
  local velocity = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(velocity, direction, magnitude)

  self:setVelocity(velocity[0])
end

function component:setVelocityQuaternion(magnitude, quaternion)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:setVelocityDirection(magnitude, direction[0])
end

function component:setAngularVelocity(angularVelocity)
  kazmath.kmVec3Assign(self.angularVel, angularVelocity)
end

function component:setAngularVelocityAxis(magnitude, axis)
  local angularVelocity = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(angularVelocity, axis, magnitude)

  self:setAngularVelocity(angularVelocity[0])
end

function component:setAngularVelocityQuaternion(magnitude, quaternion)
  local axis = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(axis, quaternion)

  self:setAngularVelocityAxis(magnitude, axis[0])
end

function component:setAcceleration(acceleration)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, acceleration, self.mass)

  self:setForce(force[0])
end

function component:setAccelerationDirection(magnitude, direction)
  local acceleration = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(acceleration, direction, magnitude)

  self:setAcceleration(acceleration[0])
end

function component:setAccelerationQuaternion(magnitude, quaternion)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:setAccelerationDirection(magnitude, direction[0])
end

function component:setForce(force)
  C.setForce(self, force)
end

function component:setForceDirection(magnitude, direction, position)
  local force = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(force, direction, magnitude)

  self:setForce(force[0])
end

function component:setForceQuaternion(magnitude, quaternion, position)
  local direction = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(direction, quaternion)

  self:setForceDirection(magnitude, direction[0])
end

function component:setTorque(torque)
  C.setTorque(self, torque)
end

function component:setTorqueAxis(magnitude, axis)
  local torque = ffi.new("kmVec3[1]")
  kazmath.kmVec3Scale(torque, axis, magnitude)

  self:setTorque(torque[0])
end

function component:setTorqueQuaternion(magnitude, quaternion)
  local axis = ffi.new("kmVec3[1]")
  kazmath.kmQuaternionGetForwardVec3RH(axis, quaternion)

  self:setTorqueAxis(magnitude, axis[0])
end

function component:zeroVelocity()
  kazmath.kmVec3Zero(self.velocity)
end

function component:zeroAngularVelocity()
  kazmath.kmVec3Zero(self.angularVel)
end

function component:zeroForce()
  local force = ffi.new("kmVec3")
  self:setForce(force)
end

function component:zeroTorque()
  local torque = ffi.new("kmVec3")
  self:setTorque(torque)
end

function component:getVelocityMagnitude()
  return kazmath.kmVec3Length(self.velocity)
end

function component:getAngularVelocityMagnitude()
  return kazmath.kmVec3Length(self.angularVel)
end

function component:getAcceleration()
  local acceleration = ffi.new("kmVec3[1]")
  acceleration[0] = self:getForce()
  kazmath.kmVec3Scale(acceleration, acceleration[0], 1.0 / self.mass)
  return acceleration[0]
end

function component:getAccelerationMagnitude()
  local acceleration = self:getAcceleration()
  return kazmath.kmVec3Length(acceleration)
end

function component:getForce()
  return C.getForce(self)
end

function component:getForceMagnitude()
  local force = self:getForce()
  return kazmath.kmVec3Length(force)
end

function component:getTorque()
  return C.getTorque(self)
end

function component:getTorqueMagnitude()
  local torque = self:getTorque()
  return kazmath.kmVec3Length(torque)
end