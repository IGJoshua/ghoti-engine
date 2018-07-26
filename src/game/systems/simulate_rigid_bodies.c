#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/rigid_body.h"
#include "components/transform.h"

#include <ode/ode.h>

#include <math.h>

internal UUID boxComponentID = {};
internal UUID sphereComponentID = {};
internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};
internal UUID hitInformationComponentID = {};
internal UUID hitListComponentID = {};
internal UUID surfaceInformationComponentID = {};
internal UUID capsuleComponentID = {};

// TODO: Move most of this stuff to rigid_body.c
// TODO: Change collisions geoms to not be a tree, but a list, not dependant on transforms

internal
void createCollisionGeom(
	Scene *scene,
	UUID entity,
	TransformComponent *trans,
	RigidBodyComponent *body,
	dSpaceID spaceID)
{
	CollisionTreeNode *node = sceneGetComponentFromEntity(
		scene,
		entity,
		collisionTreeNodeComponentID);

	ASSERT(node && "Collision tree pointed to a node with no node structure");

	switch (node->type)
	{
	case COLLISION_GEOM_TYPE_BOX:
	{
		BoxComponent *box = sceneGetComponentFromEntity(
			scene,
			entity,
			boxComponentID);

		node->geomID = dCreateBox(
			spaceID,
			box->bounds.x * trans->globalScale.x * 2,
			box->bounds.y * trans->globalScale.y * 2,
			box->bounds.z * trans->globalScale.z * 2);
	} break;
	case COLLISION_GEOM_TYPE_SPHERE:
	{
		SphereComponent *sphere = sceneGetComponentFromEntity(
			scene,
			entity,
			sphereComponentID);

		node->geomID = dCreateSphere(spaceID, sphere->radius);
	} break;
	case COLLISION_GEOM_TYPE_CAPSULE:
	{
		CapsuleComponent *capsule = sceneGetComponentFromEntity(
			scene,
			entity,
			capsuleComponentID);

		node->geomID = dCreateCapsule(spaceID, capsule->radius, capsule->length);
	} break;
	default:
	{
		ASSERT(false && "Unable to determine geometry type for entity");
	} break;
	}

	dGeomSetBody(node->geomID, body->bodyID);

	void *userData = calloc(1, sizeof(UUID));
	memcpy(userData, &entity, sizeof(UUID));
	dGeomSetData(node->geomID, userData);

	dGeomSetOffsetPosition(
		node->geomID,
		trans->position.x,
		trans->position.y,
		trans->position.z);

	dReal quat[4] = {};
	quat[0] = trans->rotation.w;
	quat[1] = trans->rotation.x;
	quat[2] = trans->rotation.y;
	quat[3] = trans->rotation.z;

	dGeomSetOffsetQuaternion(node->geomID, quat);
}

internal
void createCollisionGeoms(
	Scene *scene,
	UUID entity,
	RigidBodyComponent *body,
	dSpaceID spaceID)
{
	TransformComponent *trans = sceneGetComponentFromEntity(
		scene,
		entity,
		transformComponentID);

	if (!trans)
	{
		return;
	}

	createCollisionGeom(scene, entity, trans, body, spaceID);

	TransformComponent *child = 0;

	// Walk the tree of collision geometry
	for (UUID currentChild = trans->firstChild;
		 strcmp(currentChild.string, "");
		 currentChild = child->nextSibling)
	{
		// Add each piece of collision geometry as a different geom
		child = sceneGetComponentFromEntity(
			scene,
			currentChild,
			transformComponentID);

		createCollisionGeoms(scene, currentChild, body, spaceID);
	}
}

internal
void initSimulateRigidbodiesSystem(Scene *scene)
{
	// turn all the loaded rigidbodies into real rigidbodies
	// for each rigidbody
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetKey(
				 scene->componentTypes,
				 &rigidBodyComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		UUID entity = *cdtIteratorGetUUID(itr);
		// Create a rigidbody in the physics world
		RigidBodyComponent *body = cdtIteratorGetData(itr);
		TransformComponent *trans = sceneGetComponentFromEntity(
			scene,
			entity,
			transformComponentID);
		CollisionComponent *coll = sceneGetComponentFromEntity(
			scene,
			entity,
			collisionComponentID);

		registerRigidBody(scene, body);

		createCollisionGeoms(
			scene,
			coll->collisionTree,
			body,
			body->spaceID);

		// update all the other information about the rigidbody
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

		dBodySetMass(body->bodyID, &mass);

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
		}
		else
		{
			dBodySetKinematic(body->bodyID);
		}

		if (!body->enabled)
		{
			dBodyDisable(body->bodyID);
		}
	}
}

internal
void rigidsNearCallback(void *data, dGeomID o1, dGeomID o2)
{
	dContactGeom contacts[4] = {};

	int32 numContacts = dCollide(
		o1,
		o2,
		4,
		contacts,
		sizeof(dContactGeom));

	Scene *scene = data;

	// create contact joints, and matching hit information
	for (int32 i = 0; i < numContacts; ++i)
	{
		dContact contact = {};
		contact.geom = contacts[i];

		UUID *volume1 = dGeomGetData(o1);
		UUID *volume2 = dGeomGetData(o1);

		CollisionTreeNode *node1 = sceneGetComponentFromEntity(
			scene,
			*volume1,
			collisionTreeNodeComponentID);

		CollisionTreeNode *node2 = sceneGetComponentFromEntity(
			scene,
			*volume2,
			collisionTreeNodeComponentID);

		RigidBodyComponent *rb1 = sceneGetComponentFromEntity(
			scene,
			node1->collisionVolume,
			rigidBodyComponentID);

		RigidBodyComponent *rb2 = sceneGetComponentFromEntity(
			scene,
			node2->collisionVolume,
			rigidBodyComponentID);

		if (!rb1->isTrigger && !rb2->isTrigger)
		{
			// get surface information from the two objects
			SurfaceInformationComponent *surface1 = sceneGetComponentFromEntity(
				scene,
				*volume1,
				surfaceInformationComponentID);
			SurfaceInformationComponent *surface2 = sceneGetComponentFromEntity(
				scene,
				*volume2,
				surfaceInformationComponentID);

			SurfaceInformationComponent temp = {};
			if (surface1 && surface2)
			{
				// Combine the surface properties
				temp.bounceVelocity =
					(surface1->bounceVelocity + surface2->bounceVelocity)
					/ 2.0f;
				temp.bounciness =
					(surface1->bounciness + surface2->bounciness) / 2.0f;
				temp.disableRolling =
					surface1->disableRolling && surface2->disableRolling;
				temp.finiteFriction =
					surface1->finiteFriction && surface2->finiteFriction;
				temp.friction =
					(surface1->friction + surface2->friction) / 2.0f;
				temp.rollingFriction =
					(surface1->rollingFriction + surface2->rollingFriction)
					/ 2.0f;
				temp.spinningFriction =
					(surface1->spinningFriction + surface2->spinningFriction)
					/ 2.0f;
			}
			else if (surface1)
			{
				temp = *surface1;
			}
			else if (surface2)
			{
				temp = *surface2;
			}
			else
			{
				// Set default values
				temp.bounceVelocity = 0.01f;
				temp.bounciness = 0.2f;
				temp.disableRolling = false;
				temp.finiteFriction = true;
				temp.friction = 2;
				temp.rollingFriction = 0.1f;
				temp.spinningFriction = 0.02f;
			}

			contact.surface.mode = 0
				| (temp.disableRolling ? 0 : dContactRolling)
				| (temp.bounciness > 0 ? dContactBounce : 0);
			contact.surface.mu =
				temp.finiteFriction ? temp.friction : dInfinity;
			contact.surface.bounce = temp.bounciness;
			contact.surface.bounce_vel = temp.bounceVelocity;
			contact.surface.rho = contact.surface.rho2 = temp.rollingFriction;
			contact.surface.rhoN = temp.spinningFriction;

			dJointID joint = dJointCreateContact(
				scene->physicsWorld,
				scene->contactGroup,
				&contact);

			dJointAttach(joint, dGeomGetBody(o1), dGeomGetBody(o2));
		}

		// create hit information entities and hit list entities,
		// and attach them to the correct entities
		// create a hit_information entity
		HitInformationComponent hitInformation = {};
		hitInformation.age = 0;
		hitInformation.volume1 = *volume1;
		hitInformation.volume2 = *volume2;
		hitInformation.object1 = node1->collisionVolume;
		hitInformation.object2 = node2->collisionVolume;
		kmVec3Fill(
			&hitInformation.contactNormal,
			contact.geom.normal[0],
			contact.geom.normal[1],
			contact.geom.normal[2]);
		kmVec3Fill(
			&hitInformation.position,
			contact.geom.pos[0],
			contact.geom.pos[1],
			contact.geom.pos[2]);
		hitInformation.depth = contact.geom.depth;

		UUID hitInformationEntity = sceneCreateEntity(scene);
		sceneAddComponentToEntity(
			scene,
			hitInformationEntity,
			hitInformationComponentID,
			&hitInformation);

		// create two hit_list entities and link them to the appropriate lists
		CollisionComponent *coll1 = sceneGetComponentFromEntity(
			scene,
			hitInformation.object1,
			collisionComponentID);
		CollisionComponent *coll2 = sceneGetComponentFromEntity(
			scene,
			hitInformation.object2,
			collisionComponentID);

		HitListComponent list1 = {};
		HitListComponent list2 = {};
		list1.hit = hitInformationEntity;
		list2.hit = hitInformationEntity;
		list1.nextHit = coll1->hitList;
		list2.nextHit = coll2->hitList;

		UUID list1Entity = sceneCreateEntity(scene);
		UUID list2Entity = sceneCreateEntity(scene);
		sceneAddComponentToEntity(
			scene,
			list1Entity,
			hitListComponentID,
			&list1);
		sceneAddComponentToEntity(
			scene,
			list2Entity,
			hitListComponentID,
			&list2);

		coll1->hitList = list1Entity;
		coll2->hitList = list2Entity;
	}
}

internal
void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
	bool space1 = dGeomIsSpace(o1);
	bool space2 = dGeomIsSpace(o2);
	if (space1 || space2)
	{
		dSpaceCollide2(o1, o2, data, &rigidsNearCallback);
	}
	else
	{
		rigidsNearCallback(data, o1, o2);
	}
}

internal
void beginSimulateRigidbodiesSystem(Scene *scene, real64 dt)
{
	// TODO: Update the simulation's copy of the rigidbodies

	dSpaceCollide(scene->physicsSpace, scene, &nearCallback);

	dWorldStep(scene->physicsWorld, dt);

	dJointGroupEmpty(scene->contactGroup);
}

internal
void runSimulateRigidbodiesSystem(Scene *scene, UUID entityID, real64 dt)
{
	// update the rest of the rigidbody information
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);
	RigidBodyComponent *body = sceneGetComponentFromEntity(
		scene,
		entityID,
		rigidBodyComponentID);

	const dReal *pos = dBodyGetPosition(body->bodyID);
	transform->position.x = pos[0];
	transform->position.y = pos[1];
	transform->position.z = pos[2];

	const dReal *rot = dBodyGetQuaternion(body->bodyID);
	transform->rotation.w = rot[0];
	transform->rotation.x = rot[1];
	transform->rotation.y = rot[2];
	transform->rotation.z = rot[3];

	const dReal *vel = dBodyGetLinearVel(body->bodyID);
	body->velocity.x = vel[0];
	body->velocity.y = vel[1];
	body->velocity.z = vel[2];

	const dReal *angularVel = dBodyGetAngularVel(body->bodyID);
	body->angularVel.x = angularVel[0];
	body->angularVel.y = angularVel[1];
	body->angularVel.z = angularVel[2];

	tMarkDirty(scene, entityID);
}

internal
void freeUserData(dGeomID geom)
{
	// if it's not a space
	if (!dGeomIsSpace(geom))
	{
		free(dGeomGetData(geom));
	}
	// if it's a space
	else
	{
		dSpaceID space = (dSpaceID)geom;

		uint32 count = dSpaceGetNumGeoms(space);
		// For each geom in the space
		for (uint32 i = 0; i < count; ++i)
		{
			// free all the stuff in the user data ptr
			freeUserData(dSpaceGetGeom(space, i));
		}
	}
}

internal
void shutdownSimulateRigidbodiesSystem(Scene *scene)
{
	freeUserData((dGeomID)scene->physicsSpace);
}

System createSimulateRigidbodiesSystem(void)
{
	boxComponentID = idFromName("box");
	sphereComponentID = idFromName("sphere");
	transformComponentID = idFromName("transform");
	rigidBodyComponentID = idFromName("rigid_body");
	collisionTreeNodeComponentID = idFromName("collision_tree_node");
	collisionComponentID = idFromName("collision");
	hitInformationComponentID = idFromName("hit_information");
	hitListComponentID = idFromName("hit_list");
	surfaceInformationComponentID = idFromName("surface_information");

	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &transformComponentID);
	listPushFront(&sys.componentTypes, &rigidBodyComponentID);
	listPushFront(&sys.componentTypes, &collisionComponentID);

	sys.init = &initSimulateRigidbodiesSystem;
	sys.begin = &beginSimulateRigidbodiesSystem;
	sys.run = &runSimulateRigidbodiesSystem;
	sys.shutdown = &shutdownSimulateRigidbodiesSystem;

	return sys;
}
