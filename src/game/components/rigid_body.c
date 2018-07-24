#include "components/rigid_body.h"

#include <ode/ode.h>

RigidBodyComponent createRigidBody(Scene *scene)
{
	RigidBodyComponent ret = {};

	ret.bodyID = dBodyCreate(scene->physicsWorld);

	return ret;
}

void registerRigidBody(Scene *scene, RigidBodyComponent *body)
{
	body->bodyID = dBodyCreate(scene->physicsWorld);
}

void destroyRigidBody(RigidBodyComponent *body)
{
	dSpaceDestroy(body->spaceID);
	dBodyDestroy(body->bodyID);
}
