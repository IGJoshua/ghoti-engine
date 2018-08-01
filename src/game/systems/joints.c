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

#include <ode/ode.h>

#include <string.h>

internal UUID jointListComponentID = {};
internal UUID jointInfoComponentID = {};
internal UUID rigidBodyComponentID = {};

internal UUID jointHingeComponentID = {};
internal UUID jointSliderComponentID = {};
internal UUID jointBallSocketComponentID = {};


internal
void initJointInformationSystem(Scene *scene)
{
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetKey(
				 scene->componentTypes,
				 &jointInfoComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		UUID entityID = *cdtIteratorGetUUID(itr);

		JointInformationComponent *joint = cdtIteratorGetData(itr);

		switch (joint->type)
		{
		case JOINT_TYPE_SLIDER:
		{

		} break;
		case JOINT_TYPE_HINGE:
		{
			HingeJointComponent *hJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointHingeComponentID);

			hJoint->hinge = dJointCreateHinge(scene->physicsWorld, 0);

			dJointSetHingeAnchor(
				hJoint->hinge,
				hJoint->anchor.x,
				hJoint->anchor.y,
				hJoint->anchor.z);

			dJointSetHingeAxis(
				hJoint->hinge,
				hJoint->axis.x,
				hJoint->axis.y,
				hJoint->axis.z);

		} break;
		case JOINT_TYPE_BALL_SOCKET:
		{

		} break;
		default:
		{
			ASSERT(false && "Unable to determine joint type");
		}break;
		}
	}
}

internal
void runJointInformationSystem(Scene *scene, UUID entityID, real64 dt)
{

/*
	JointListComponent *jointList = sceneGetComponentFromEntity(
		scene,
		entityID,
		jointListComponentID);

	JointInformationComponent *joint = sceneGetComponentFromEntity(
		scene,
		jointList->jointInfo,
		jointInfoComponentID);
	RigidBodyComponent *object1 = sceneGetComponentFromEntity(
		scene,
		joint->object1,
		rigidBodyComponentID);

	RigidBodyComponent *object2 = sceneGetComponentFromEntity(
		scene,
		joint->object2,
		rigidBodyComponentID);
	switch (joint->type)
	{
	case JOINT_TYPE_HINGE:
	{


	} break;
	case JOINT_TYPE_SLIDER:
	{

	} break;
	case JOINT_TYPE_BALL_SOCKET:
	{

	} break;
	default:
	{
		ASSERT(false && "Unable to determine joint type");
	}break;
	}

*/

}

System createJointInformationSystem(void)
{
	jointListComponentID = idFromName("joint_list");
	jointInfoComponentID = idFromName("joint_information");
	rigidBodyComponentID = idFromName("rigid_body");

	jointHingeComponentID = idFromName("joint_hinge");
	jointSliderComponentID = idFromName("joint_slider");
	jointBallSocketComponentID = idFromName("joint_ball_socket");


	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &jointListComponentID);

	sys.init = initJointInformationSystem;
	sys.begin = 0;
	sys.run = &runJointInformationSystem;
	sys.end = 0;
	sys.shutdown = 0;

	return sys;
}
