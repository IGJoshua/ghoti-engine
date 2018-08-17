ffi.cdef[[
void registerRigidBody(Scene *scene, UUID entity);
void destroyRigidBody(RigidBodyComponent *body);

void updateRigidBodyPosition(
  Scene *scene,
  CollisionComponent *coll,
  RigidBodyComponent *body,
  TransformComponent *trans);
void updateRigidBody(
  Scene *scene,
  CollisionComponent *coll,
  RigidBodyComponent *body,
  TransformComponent *trans);
]]
