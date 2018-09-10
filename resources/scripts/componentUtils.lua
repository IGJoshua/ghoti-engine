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
	AnimatorComponent *animator,
	const char *name,
	int32 loopCount,
	real32 speed,
	real32 transitionDuration,
	bool stopPreviousAnimation);
void stopAnimation(AnimatorComponent *animator);
]]
