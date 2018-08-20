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

internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};
internal UUID hitInformationComponentID = {};
internal UUID hitListComponentID = {};
internal UUID surfaceInformationComponentID = {};

internal
void initSimulateRigidbodiesSystem(Scene *scene)
{
	// turn all the loaded rigidbodies into real rigidbodies
	// for each rigidbody
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &rigidBodyComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		// Create a rigidbody in the physics world
		registerRigidBody(scene, cdtIteratorGetUUID(itr));
	}
}

internal
void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
	Scene *scene = data;

	bool space1 = dGeomIsSpace(o1);
	bool space2 = dGeomIsSpace(o2);
	if (space1 || space2)
	{
		dSpaceCollide2(o1, o2, scene, &nearCallback);
	}
	else
	{
		// TODO: Make this work using indices into the tables instead of UUIDs
		UUID *volume1 = dGeomGetData(o1);
		UUID *volume2 = dGeomGetData(o2);

		CollisionTreeNode *node1 = sceneGetComponentFromEntity(
			scene,
			*volume1,
			collisionTreeNodeComponentID);

		CollisionTreeNode *node2 = sceneGetComponentFromEntity(
			scene,
			*volume2,
			collisionTreeNodeComponentID);

		RigidBodyComponent *body1 = sceneGetComponentFromEntity(
			scene,
			node1->collisionVolume,
			rigidBodyComponentID);

		RigidBodyComponent *body2 = sceneGetComponentFromEntity(
			scene,
			node2->collisionVolume,
			rigidBodyComponentID);

		// FIXME: This should not collide if both bodies are kinematic
		if ((node1->isTrigger && node2->isTrigger)
			|| ((body1 && !body1->dynamic) && (body2 && !body2->dynamic)))
		{
			return;
		}

		dContactGeom contacts[4] = {};

		int32 numContacts = dCollide(
			o1,
			o2,
			4,
			contacts,
			sizeof(dContactGeom));

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

		if (numContacts == 0)
		{
			return;
		}

		// create contact joints, and matching hit information
		for (int32 i = 0; i < numContacts; ++i)
		{
			dContact contact = {};
			contact.geom = contacts[i];

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

		dContact contact = {};
		contact.geom = contacts[0];

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
void beginSimulateRigidbodiesSystem(Scene *scene, real64 dt)
{
	// Update the simulation's copy of the rigidbodies
	TransformComponent *trans = 0;
	RigidBodyComponent *body = 0;
	CollisionComponent *coll = 0;
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &rigidBodyComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		body = (RigidBodyComponent *)cdtIteratorGetData(itr);

		trans = sceneGetComponentFromEntity(
			scene,
			cdtIteratorGetUUID(itr),
			transformComponentID);

		coll = sceneGetComponentFromEntity(
			scene,
			cdtIteratorGetUUID(itr),
			collisionComponentID);

		updateRigidBodyPosition(
			scene,
			coll,
			body,
			trans);
	}

	dSpaceCollide(scene->physicsSpace, scene, &nearCallback);
	dWorldQuickStep(scene->physicsWorld, dt);
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
void shutdownSimulateRigidbodiesSystem(Scene *scene)
{
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &rigidBodyComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		destroyRigidBody(cdtIteratorGetData(itr));
	}
}

System createSimulateRigidbodiesSystem(void)
{
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

	sys.init = &initSimulateRigidbodiesSystem;
	sys.begin = &beginSimulateRigidbodiesSystem;
	sys.run = &runSimulateRigidbodiesSystem;
	sys.shutdown = &shutdownSimulateRigidbodiesSystem;

	return sys;
}
