#pragma once
#include "defines.h"

#include <kazmath/vec3.h>
#include <kazmath/quaternion.h>

typedef struct model_component_t
{
	char name[1024];
} ModelComponent;

typedef struct transform_component_t
{
	kmQuaternion rotation;
	kmVec3 position;
	kmVec3 scale;
} TransformComponent;
