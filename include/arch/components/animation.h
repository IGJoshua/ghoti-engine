#include "defines.h"

#include "components/component_types.h"

#include "ECS/ecs_types.h"

#define SKELETON_BUCKET_COUNT 257

typedef struct joint_transform_t
{
	TransformComponent *transform;
	UUID uuid;
} JointTransform;

void addSkeleton(Scene *scene, UUID skeletonID);