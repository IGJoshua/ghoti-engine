#include "defines.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

typedef struct ray_collision_t
{
    bool hasContact;
    real32 contact_pos[3];
	real32 surface_normal[3];
	real32 distance;
    real32 minDist;
    UUID contact_UUID;
} RayCollision;

RayCollision rayCast(
    Scene *scene,
    kmVec3 pos,
    kmVec3 dir,
    real32 minDist,
    real32 length);
