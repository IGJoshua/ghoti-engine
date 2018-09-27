ffi.cdef[[

void registerRigidBody(Scene *scene, UUID entity);
void createCollisionGeoms(
	Scene *scene,
	TransformComponent *bodyTrans,
	RigidBodyComponent *body,
	CollisionComponent *coll);

void addForce(RigidBodyComponent *body, kmVec3 *force, kmVec3 *position);
void addTorque(RigidBodyComponent *body, kmVec3 *torque);

void setForce(RigidBodyComponent *body, kmVec3 *force);
void setTorque(RigidBodyComponent *body, kmVec3 *torque);

kmVec3 getForce(RigidBodyComponent *body);
kmVec3 getTorque(RigidBodyComponent *body);

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

void destroyRigidBody(RigidBodyComponent *body);

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

void emitParticles(
	ParticleEmitterComponent *particleEmitter,
	bool stopAtCapacity,
	const char *particleName,
	uint32 numSprites,
	int32 spriteWidth,
	int32 spriteHeight,
	int32 initialSprite,
	bool randomSprite,
	real32 animationFPS,
	bool loop);
void stopParticleEmitter(ParticleEmitterComponent *particleEmitter, bool reset);

]]
