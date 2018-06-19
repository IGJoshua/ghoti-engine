#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

typedef struct model_component_t
{
	char name[1025];
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
	float nearPlane;
	float farPlane;
	float aspectRatio;
	float fov;
	CameraProjectionType projectionType;
} CameraComponent;
