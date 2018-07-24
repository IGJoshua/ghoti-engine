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
internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};

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
			box->bounds.x,
			box->bounds.y,
			box->bounds.z);
	} break;
	case COLLISION_GEOM_TYPE_SPHERE:
	{
	} break;
	default:
	{
		ASSERT(false && "Unable to determine geometry type for entity");
	} break;
	}

	dGeomSetBody(node->geomID, body->bodyID);

	dGeomSetOffsetPosition(
		node->geomID,
		trans->position.x,
		trans->position.y,
		trans->position.z);

	dQuaternion quat;
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

	createCollisionGeom(scene, entity, trans, body, spaceID);

	if (!trans)
	{
		return;
	}

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

		// TODO: Move this to registerRigidBody
		body->spaceID = dSimpleSpaceCreate(scene->physicsSpace);

		createCollisionGeoms(
			scene,
			coll->collisionTree,
			body,
			body->spaceID);

		// TODO: update all the other information about the rigidbody
		dMass mass;
		real32 aabb[6];
		dGeomGetAABB((dGeomID)body->spaceID, aabb);

		real32 radius = fmaxf(
			aabb[1] - aabb[0], fmaxf(aabb[3] - aabb[2], aabb[5] - aabb[4]))
			/ 2.0f;

		dMassSetSphereTotal(&mass, body->mass, radius);
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

		dQuaternion rot;
		rot[0] = trans->globalRotation.w;
		rot[1] = trans->globalRotation.x;
		rot[2] = trans->globalRotation.y;
		rot[3] = trans->globalRotation.z;

		dBodySetRotation(body->bodyID, rot);

		if (body->dynamic)
		{
			dBodySetDynamic(body->bodyID);
		}
		else
		{
			dBodySetKinematic(body->bodyID);
		}
	}
}

internal
void nearCallback(void *data, dGeomID o1, dGeomID o2)
{

}

internal
void beginSimulateRigidbodiesSystem(Scene *scene, real64 dt)
{
	// run a physics frame
	dWorldStep(scene->physicsWorld, dt);

	// TODO: check for collisions and add them to a joint group
	dSpaceCollide(scene->physicsSpace, 0, &nearCallback);
}

internal
void runSimulateRigidbodiesSystem(Scene *scene, UUID entityID, real64 dt)
{
	// TODO: update the rigidbody information
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

	tMarkDirty(scene, entityID);
}

internal
void endSimulateRigidbodiesSystem(Scene *scene, real64 dt)
{
}

internal
void shutdownSimulateRigidbodiesSystem(Scene *scene)
{
	// TODO: shutdown all the ode stuff
}

System createSimulateRigidbodiesSystem(void)
{
	boxComponentID = idFromName("box");
	transformComponentID = idFromName("transform");
	rigidBodyComponentID = idFromName("rigid_body");
	collisionTreeNodeComponentID = idFromName("collision_tree_node");
	collisionComponentID = idFromName("collision");
	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &transformComponentID);
	listPushFront(&sys.componentTypes, &rigidBodyComponentID);
	listPushFront(&sys.componentTypes, &collisionComponentID);

	sys.init = &initSimulateRigidbodiesSystem;
	sys.begin = &beginSimulateRigidbodiesSystem;
	sys.run = &runSimulateRigidbodiesSystem;
	sys.end = &endSimulateRigidbodiesSystem;
	sys.shutdown = &shutdownSimulateRigidbodiesSystem;

	return sys;
}
