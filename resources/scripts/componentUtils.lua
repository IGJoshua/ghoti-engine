ffi.cdef[[
void registerRigidBody(Scene *scene, UUID entity);
void destroyRigidBody(RigidBodyComponent *body);

void updateRigidBodyPosition(
  RigidBodyComponent *body,
  TransformComponent *trans);
void updateRigidBody(
  RigidBodyComponent *body,
  TransformComponent *trans);
]]
