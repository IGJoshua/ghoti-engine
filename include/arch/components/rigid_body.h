#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

void registerRigidBody(Scene *scene, UUID entity);
void destroyRigidBody(RigidBodyComponent *body);

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

void freeRigidBody(UUID componentID, ComponentDataEntry *entry);