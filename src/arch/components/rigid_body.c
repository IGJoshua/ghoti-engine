#include "components/rigid_body.h"

#include "components/transform.h"

#include "core/log.h"

#include <ode/ode.h>

internal
void updateCollisionGeom(
	TransformComponent *bodyTrans,
	TransformComponent *trans,
	CollisionTreeNode *node)
{
	kmVec3 pos, scal;
	kmQuaternion rot;
	tGetInverseGlobalTransform(bodyTrans, &pos, &rot, &scal);
	kmVec3Add(&pos, &trans->globalPosition, &pos);
	kmQuaternionMultiply(&rot, &rot, &trans->globalRotation);
	kmVec3Mul(&scal, &scal, &trans->globalScale);

	dGeomSetOffsetPosition(
		node->geomID,
		pos.x,
		pos.y,
		pos.z);

	dReal quat[4] = {};
	quat[0] = rot.w;
	quat[1] = rot.x;
	quat[2] = rot.y;
	quat[3] = rot.z;

	dGeomSetOffsetQuaternion(node->geomID, quat);
}

void updateCollisionGeoms(
	Scene *scene,
	TransformComponent *bodyTrans,
	CollisionComponent *coll)
{
	CollisionTreeNode *node = 0;
	TransformComponent *trans = 0;
	UUID collisionTreeNodeComponentID = idFromName("collision_tree_node");
	UUID transformComponentID = idFromName("transform");

	// Walk the tree of collision geometry
	for (UUID currentCollider = coll->collisionTree;
		 strcmp(currentCollider.string, "");
		 currentCollider = node->nextCollider)
	{
		trans = sceneGetComponentFromEntity(
			scene,
			currentCollider,
			transformComponentID);

		node = sceneGetComponentFromEntity(
			scene,
			currentCollider,
			collisionTreeNodeComponentID);

		// Add each piece of collision geometry as a different geom
		updateCollisionGeom(bodyTrans, trans, node);
	}
}

internal
void createCollisionGeom(
	Scene *scene,
	UUID entity,
	TransformComponent *bodyTrans,
	RigidBodyComponent *body,
	dSpaceID spaceID)
{
	CollisionTreeNode *node = sceneGetComponentFromEntity(
		scene,
		entity,
		idFromName("collision_tree_node"));
	TransformComponent *trans = sceneGetComponentFromEntity(
		scene,
		entity,
		idFromName("transform"));

	ASSERT(node && "Collision tree pointed to a node with no node structure");

	real32 maxScale = MAX(
		MAX(trans->globalScale.x,
			trans->globalScale.y),
		trans->globalScale.z);

	switch (node->type)
	{
	case COLLISION_GEOM_TYPE_BOX:
	{
		BoxComponent *box = sceneGetComponentFromEntity(
			scene,
			entity,
			idFromName("box"));

		node->geomID = dCreateBox(
			spaceID,
			box->bounds.x * trans->globalScale.x * 2,
			box->bounds.y * trans->globalScale.y * 2,
			box->bounds.z * trans->globalScale.z * 2);
		dGeomSetQuaternion(node->geomID, (dReal*)&trans->globalRotation);
	} break;
	case COLLISION_GEOM_TYPE_SPHERE:
	{
		SphereComponent *sphere = sceneGetComponentFromEntity(
			scene,
			entity,
			idFromName("sphere"));

		node->geomID = dCreateSphere(spaceID, sphere->radius * maxScale);
	} break;
	case COLLISION_GEOM_TYPE_CAPSULE:
	{
		CapsuleComponent *capsule = sceneGetComponentFromEntity(
			scene,
			entity,
			idFromName("capsule"));

		node->geomID = dCreateCapsule(
			spaceID,
			capsule->radius * maxScale,
			capsule->length * maxScale);
		dGeomSetQuaternion(node->geomID, (dReal*)&trans->globalRotation);
	} break;
	default:
	{
		ASSERT(false && "Unable to determine geometry type for entity");
	} break;
	}

	dGeomSetBody(node->geomID, body->bodyID);

	// TODO: Store the index in the component data table, instead of the uuid
	void *userData = calloc(1, sizeof(UUID));
	memcpy(userData, &entity, sizeof(UUID));
	dGeomSetData(node->geomID, userData);

	updateCollisionGeom(bodyTrans, trans, node);
}

internal
void createCollisionGeoms(
	Scene *scene,
	TransformComponent *bodyTrans,
	RigidBodyComponent *body,
	CollisionComponent *coll,
	dSpaceID spaceID)
{
	CollisionTreeNode *node = 0;
	UUID collisionTreeNodeID = idFromName("collision_tree_node");

	// Walk the tree of collision geometry
	for (UUID currentCollider = coll->collisionTree;
		 strcmp(currentCollider.string, "");
		 currentCollider = node->nextCollider)
	{
		// Add each piece of collision geometry as a different geom
		createCollisionGeom(scene, currentCollider, bodyTrans, body, spaceID);

		node = sceneGetComponentFromEntity(
			scene,
			currentCollider,
			collisionTreeNodeID);
	}
}

void registerRigidBody(Scene *scene, UUID entity)
{
	RigidBodyComponent *body = sceneGetComponentFromEntity(
		scene,
		entity,
		idFromName("rigid_body"));
	CollisionComponent *coll = sceneGetComponentFromEntity(
		scene,
		entity,
		idFromName("collision"));
	TransformComponent *trans = sceneGetComponentFromEntity(
		scene,
		entity,
		idFromName("transform"));

	body->bodyID = dBodyCreate(scene->physicsWorld);
	body->spaceID = dSimpleSpaceCreate(scene->physicsSpace);

	if (coll)
	{
		createCollisionGeoms(
			scene,
			trans,
			body,
			coll,
			body->spaceID);
	}

	// update all the other information about the rigidbody
	updateRigidBody(scene, coll, body, trans);
}

void destroyRigidBody(RigidBodyComponent *body)
{
	dSpaceDestroy(body->spaceID);
	dBodyDestroy(body->bodyID);
}

void updateRigidBodyPosition(
	Scene *scene,
	CollisionComponent *coll,
	RigidBodyComponent *body,
	TransformComponent *trans)
{
	dBodySetAngularVel(
		body->bodyID,
		body->angularVel.x,
		body->angularVel.y,
		body->angularVel.z);

	dBodySetLinearVel(
		body->bodyID,
		body->velocity.x,
		body->velocity.y,
		body->velocity.z);

	dBodySetPosition(
		body->bodyID,
		trans->globalPosition.x,
		trans->globalPosition.y,
		trans->globalPosition.z);

	dQuaternion rot = {};
	rot[0] = trans->globalRotation.w;
	rot[1] = trans->globalRotation.x;
	rot[2] = trans->globalRotation.y;
	rot[3] = trans->globalRotation.z;

	dBodySetQuaternion(body->bodyID, rot);

	if (coll)
	{
		updateCollisionGeoms(scene, trans, coll);
	}
}

void updateRigidBody(
	Scene *scene,
	CollisionComponent *coll,
	RigidBodyComponent *body,
	TransformComponent *trans)
{
	dMass mass = {};

	switch (body->inertiaType)
	{
	case MOMENT_OF_INERTIA_USER:
	{
		dMassSetParameters(
			&mass,
			body->mass,
			body->centerOfMass.x,
			body->centerOfMass.y,
			body->centerOfMass.z,
			body->moiParams[0],
			body->moiParams[1],
			body->moiParams[2],
			body->moiParams[3],
			body->moiParams[4],
			body->moiParams[5]);
	} break;
	case MOMENT_OF_INERTIA_CUBE:
	{
		dMassSetBoxTotal(
			&mass,
			body->mass,
			body->moiParams[0] * 2,
			body->moiParams[1] * 2,
			body->moiParams[2] * 2);
	} break;
	case MOMENT_OF_INERTIA_SPHERE:
	{
		dMassSetSphereTotal(&mass, body->mass, body->moiParams[0]);
	} break;
	case MOMENT_OF_INERTIA_CAPSULE:
	{
		dMassSetCapsuleTotal(
			&mass,
			body->mass,
			roundf(body->moiParams[0]),
			body->moiParams[1],
			body->moiParams[2]);
	} break;
	case MOMENT_OF_INERTIA_DEFAULT:
	default:
	{
		real32 aabb[6] = {};
		dGeomGetAABB((dGeomID)body->spaceID, aabb);

		real32 radius = fmaxf(
			aabb[1] - aabb[0], fmaxf(aabb[3] - aabb[2], aabb[5] - aabb[4]))
			/ 2.0f;

		dMassSetSphereTotal(&mass, body->mass, radius);
	} break;
	}

	if (body->inertiaType != MOMENT_OF_INERTIA_USER)
	{
		dMassTranslate(
			&mass,
			body->centerOfMass.x,
			body->centerOfMass.y,
			body->centerOfMass.z);
	}

	if (body->gravity)
	{
		dBodySetGravityMode(body->bodyID, 1);
	}
	else
	{
		dBodySetGravityMode(body->bodyID, 0);
	}

	if (!body->defaultDamping)
	{
		dBodySetLinearDamping(body->bodyID, body->linearDamping);
		dBodySetAngularDamping(body->bodyID, body->angularDamping);

		dBodySetLinearDampingThreshold(
			body->bodyID,
			body->linearDampingThreshold);
		dBodySetAngularDampingThreshold(
			body->bodyID,
			body->angularDampingThreshold);
	}

	dBodySetMaxAngularSpeed(body->bodyID, body->maxAngularSpeed);

	if (body->dynamic)
	{
		dBodySetDynamic(body->bodyID);
		dBodySetMass(body->bodyID, &mass);
	}
	else
	{
		dBodySetKinematic(body->bodyID);
	}

	updateRigidBodyPosition(scene, coll, body, trans);

	if (body->enabled)
	{
		dBodyEnable(body->bodyID);
	}
	else
	{
		dBodyDisable(body->bodyID);
	}
}
