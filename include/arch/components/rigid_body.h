#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

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

void updateRigidBody(
	Scene *scene,
	CollisionComponent *coll,
	RigidBodyComponent *body,
	TransformComponent *trans);
void updateRigidBodyPosition(
	Scene *scene,
	CollisionComponent *coll,
	RigidBodyComponent *body,
	TransformComponent *trans);

void destroyRigidBody(RigidBodyComponent *body);