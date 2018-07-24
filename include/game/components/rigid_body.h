#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "component_types.h"

RigidBodyComponent createRigidBody(Scene *scene);
void registerRigidBody(Scene *scene, RigidBodyComponent *body);
void destroyRigidBody(RigidBodyComponent *body);
