#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "component_types.h"

void registerRigidBody(Scene *scene, UUID entity, RigidBodyComponent *body);
void destroyRigidBody(RigidBodyComponent *body);
