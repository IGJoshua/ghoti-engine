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

void playAnimation(
	ModelComponent *modelComponent,
	AnimationComponent *animationComponent,
	const char *name,
	bool loop,
	real32 speed,
	bool backwards);
void stopAnimation(AnimationComponent *animationComponent);
]]
