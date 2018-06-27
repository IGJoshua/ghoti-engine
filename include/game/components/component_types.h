#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

typedef struct model_component_t
{
	char name[1024];
	bool visible;
} ModelComponent;

typedef struct transform_component_t
{
	kmVec3 position;
	kmQuaternion rotation;
	kmVec3 scale;
	UUID parent;
	UUID firstChild;
	UUID nextSibling;
	bool dirty;
	kmVec3 globalPosition;
	kmQuaternion globalRotation;
	kmVec3 globalScale;
	kmVec3 lastGlobalPosition;
	kmQuaternion lastGlobalRotation;
	kmVec3 lastGlobalScale;
} TransformComponent;

typedef enum camera_projection_type_e
{
	CAMERA_PROJECTION_TYPE_PERSPECTIVE = 0,
	CAMERA_PROJECTION_TYPE_ORTHOGRAPHIC,
	CAMERA_PROJECTION_TYPE_COUNT
} CameraProjectionType;

typedef struct camera_component_t
{
	real32 nearPlane;
	real32 farPlane;
	real32 aspectRatio;
	real32 fov;
	CameraProjectionType projectionType;
} CameraComponent;

typedef struct collision_component_t
{
	UUID collisionTree;
	UUID hitList;
	UUID lastHitList;
} CollisionComponent;

typedef enum bounding_volume_type_e
{
	BOUNDING_VOLUME_TYPE_AABB = 0,
	BOUNDING_VOLUME_TYPE_COUNT
} BoundingVolumeType;

typedef struct collision_tree_node_component_t
{
	UUID parent;
	UUID nextSibling;
	UUID firstChild;
	BoundingVolumeType volumeType;
} CollisionTreeNodeComponent;

typedef struct aabb_component_t
{
	UUID collisionVolume;
	kmVec3 bounds;
} AABBComponent;

typedef struct hit_information_component_t
{
	UUID otherObject;
	UUID nextHit;
} HitInformationComponent;
