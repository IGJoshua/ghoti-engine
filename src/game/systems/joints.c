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
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &jointInfoComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		UUID entityID = cdtIteratorGetUUID(itr);

		JointInformationComponent *joint = cdtIteratorGetData(itr);

		RigidBodyComponent *object1 = sceneGetComponentFromEntity(
			scene,
			joint->object1,
			rigidBodyComponentID);

		RigidBodyComponent *object2 = sceneGetComponentFromEntity(
			scene,
			joint->object2,
			rigidBodyComponentID);

		if(!(object1 || object2))
			continue;

		printf("Joint UUID: %s\n", entityID.string);
		printf("Object1 UUID: %s\n", joint->object1.string);
		printf("Object2 UUID: %s\n", joint->object2.string);

		dJointID jointID = 0;

		// TODO: Create all the joints
		switch (joint->type)
		{
		case JOINT_TYPE_SLIDER:
		{
			ASSERT(false && "Slider joint type not yet supported.\n");
		} break;
		case JOINT_TYPE_HINGE:
		{
			HingeJointComponent *hJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointHingeComponentID);

			hJoint->hinge = dJointCreateHinge(scene->physicsWorld, 0);

			jointID = hJoint->hinge;
		} break;
		default:
		{
			LOG("Invalid joint type added on entity %s\n", entityID.string);
		} break;
		}

		// Attach all the joints to the relevant objects
		if(object1 && object2)
		{
			dJointAttach(jointID, object1->bodyID, object2->bodyID);
		}
		else if(object1)
		{
			dJointAttach(jointID, object1->bodyID, 0);
		}
		else
		{
			dJointAttach(jointID, 0, object2->bodyID);
		}

		// TODO: Set all the joint-specific values
		switch (joint->type)
		{
		case JOINT_TYPE_SLIDER:
		{
			printf("%s\n", "\n\nSlider?\n\n");

		} break;
		case JOINT_TYPE_HINGE:
		{
			HingeJointComponent *hJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointHingeComponentID);

			printf("\nAnchor:\n");
			printf("x: %f\n", hJoint->anchor.x);
			printf("y: %f\n", hJoint->anchor.y);
			printf("z: %f\n", hJoint->anchor.z);

			printf("\nAxis:\n");
			printf("x: %f\n", hJoint->axis.x);
			printf("y: %f\n", hJoint->axis.y);
			printf("z: %f\n", hJoint->axis.z);

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

			dReal Vec3[3] = {};

			dJointGetHingeAnchor(hJoint->hinge, Vec3);

			printf("\nAnchor:\n");
			printf("x: %f\n", Vec3[0]);
			printf("y: %f\n", Vec3[1]);
			printf("z: %f\n", Vec3[2]);

			dJointGetHingeAxis(hJoint->hinge, Vec3);

			printf("\nAxis:\n");
			printf("x: %f\n", Vec3[0]);
			printf("y: %f\n", Vec3[1]);
			printf("z: %f\n", Vec3[2]);
		} break;
		case JOINT_TYPE_BALL_SOCKET:
		{
			printf("%s\n", "\n\nBall Socket?\n\n");

		} break;
		default:
		{
			printf("%s\n", "\n\nDEFAULT!?\n\n");

		}break;
		}

		dJointEnable(jointID);
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

	jointHingeComponentID = idFromName("hinge_joint");
	jointSliderComponentID = idFromName("slider_joint");
	jointBallSocketComponentID = idFromName("ball_socket_joint");


	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &jointInfoComponentID);

	sys.init = initJointInformationSystem;
	sys.begin = 0;
	sys.run = &runJointInformationSystem;
	sys.end = 0;
	sys.shutdown = 0;

	return sys;
}
