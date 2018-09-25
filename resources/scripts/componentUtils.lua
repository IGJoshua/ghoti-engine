ffi.cdef[[

void registerRigidBody(Scene *scene, UUID entity);
void createCollisionGeoms(
	Scene *scene,
	TransformComponent *bodyTrans,
	RigidBodyComponent *body,
	CollisionComponent *coll);
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

void removeSkeleton(Scene *scene, UUID skeletonID);

void resetAnimator(
	AnimatorComponent *animator,
	void *animationReference);
void removeAnimator(AnimatorComponent *animator);

void playSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName);
void pauseSoundAtSource(AudioSourceComponent *audioSource);
void resumeSoundAtSource(AudioSourceComponent *audioSource);
void stopSoundAtSource(AudioSourceComponent *audioSource);
bool isSourceActive(AudioSourceComponent *audioSource);

]]
