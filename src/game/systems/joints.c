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

internal UUID transformComponentID = {};
internal UUID jointListComponentID = {};
internal UUID jointInfoComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID jointConstraintComponentID = {};

internal UUID jointHingeComponentID = {};
internal UUID jointSliderComponentID = {};
internal UUID jointBallSocketComponentID = {};
internal UUID jointBallSocket2ComponentID = {};

internal
void setJointConstraints(
	dJointID jointID,
	JointConstraintComponent* constraint,
	void(*ParamFn)(dJointID, int, dReal))
{
	//Set any optional parameters
	if (constraint)
	{
		if (constraint->loStop_bool)
		{
			ParamFn(jointID, dParamLoStop, constraint->loStop_val);
		}

		if (constraint->hiStop_bool)
		{
			ParamFn(jointID, dParamHiStop, constraint->hiStop_val);
		}

		if (constraint->bounce_bool)
		{
			ParamFn(jointID, dParamBounce, constraint->bounce_val);
		}

		if (constraint->CFM_bool)
		{
			ParamFn(jointID, dParamCFM, constraint->CFM_val);
		}

		if (constraint->stopERP_bool)
		{
			ParamFn(jointID, dParamStopERP, constraint->stopERP_val);
		}

		if (constraint->stopCFM_bool)
		{
			ParamFn(jointID, dParamStopCFM, constraint->stopCFM_val);
		}

		LOG("constraints set\n\n");
	}
	else
	{
		LOG("constraints not set\n\n");
	}
}

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

		JointConstraintComponent *constraint = sceneGetComponentFromEntity(
			scene,
			entityID,
			jointConstraintComponentID);

		RigidBodyComponent *object1 = sceneGetComponentFromEntity(
			scene,
			joint->object1,
			rigidBodyComponentID);

		RigidBodyComponent *object2 = sceneGetComponentFromEntity(
			scene,
			joint->object2,
			rigidBodyComponentID);

		if (!(object1 || object2))
			continue;

		LOG("Joint UUID: %s\n", entityID.string);
		LOG("Object1 UUID: %s\n", joint->object1.string);
		LOG("Object2 UUID: %s\n", joint->object2.string);

		dJointID jointID = 0;

		switch (joint->type)
		{
		case JOINT_TYPE_SLIDER:
		{
			SliderJointComponent *sJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointSliderComponentID);

			sJoint->id = dJointCreateSlider(scene->physicsWorld, 0);

			jointID = sJoint->id;

		} break;
		case JOINT_TYPE_HINGE:
		{
			HingeJointComponent *hJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointHingeComponentID);

			hJoint->id = dJointCreateHinge(scene->physicsWorld, 0);

			jointID = hJoint->id;

		} break;
		case JOINT_TYPE_BALL_SOCKET:
		{
			BallSocketJointComponent *bJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointBallSocketComponentID);

			bJoint->id = dJointCreateBall(scene->physicsWorld, 0);

			jointID =bJoint->id;

		} break;
		case JOINT_TYPE_BALL_SOCKET2:
		{
			BallSocket2JointComponent *b2Joint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointBallSocket2ComponentID);

			b2Joint->id = dJointCreateDBall(scene->physicsWorld, 0);

			jointID =b2Joint->id;

		} break;
		default:
		{
			LOG("Invalid joint type added on entity %s\n", entityID.string);
		} break;
		}

		// Attach all the joints to the relevant objects
		if (object1 && object2)
		{
			dJointAttach(jointID, object1->bodyID, object2->bodyID);
		}
		else if (object1)
		{
			dJointAttach(jointID, object1->bodyID, 0);
		}
		else
		{
			dJointAttach(jointID, 0, object2->bodyID);
		}

		switch (joint->type)
		{
		case JOINT_TYPE_SLIDER:
		{
			SliderJointComponent *sJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointSliderComponentID);

			dJointSetSliderAxis(
				sJoint->id,
				sJoint->axis.x,
				sJoint->axis.y,
				sJoint->axis.z);

			setJointConstraints(jointID, constraint, dJointSetSliderParam);

		} break;
		case JOINT_TYPE_HINGE:
		{
			HingeJointComponent *hJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointHingeComponentID);

			dJointSetHingeAnchor(
				hJoint->id,
				hJoint->anchor.x,
				hJoint->anchor.y,
				hJoint->anchor.z);

			dJointSetHingeAxis(
				hJoint->id,
				hJoint->axis.x,
				hJoint->axis.y,
				hJoint->axis.z);

			setJointConstraints(jointID, constraint, dJointSetHingeParam);

		} break;
		case JOINT_TYPE_BALL_SOCKET:
		{
			BallSocketJointComponent *bJoint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointBallSocketComponentID);

			dJointSetBallAnchor(
				bJoint->id,
				bJoint->anchor.x,
				bJoint->anchor.y,
				bJoint->anchor.z);

		} break;
		case JOINT_TYPE_BALL_SOCKET2:
		{
			BallSocket2JointComponent *b2Joint = sceneGetComponentFromEntity(
				scene,
				entityID,
				jointBallSocket2ComponentID);

			dJointSetDBallAnchor1(
				b2Joint->id,
				b2Joint->anchor1.x,
				b2Joint->anchor1.y,
				b2Joint->anchor1.z);

			dJointSetDBallAnchor2(
				b2Joint->id,
				b2Joint->anchor2.x,
				b2Joint->anchor2.y,
				b2Joint->anchor2.z);

			dJointSetDBallDistance(
				b2Joint->id,
				b2Joint->distance);

			setJointConstraints(jointID, constraint, dJointSetDBallParam);

		} break;
		default:
		{
			LOG("%s\n", "\n\nDEFAULT!?\n\n");
		}break;
		}

		dJointEnable(jointID);
	}
}

internal
void runJointInformationSystem(Scene *scene, UUID entityID, real64 dt)
{
	TransformComponent *trans = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	if (!trans)
		return;

	LOG("Entity %s has position (%f, %f, %f)\n",
		entityID.string,
		trans->position.x,
		trans->position.y,
		trans->position.z);

	return;
}

System createJointInformationSystem(void)
{
	transformComponentID = idFromName("transform");
	jointListComponentID = idFromName("joint_list");
	jointInfoComponentID = idFromName("joint_information");
	rigidBodyComponentID = idFromName("rigid_body");
	jointConstraintComponentID = idFromName("joint_constraint");

	jointHingeComponentID = idFromName("hinge_joint");
	jointSliderComponentID = idFromName("slider_joint");
	jointBallSocketComponentID = idFromName("ball_socket_joint");
	jointBallSocket2ComponentID = idFromName("ball_socket2_joint");

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
