#pragma once
#include "defines.h"

#include "components/component_types.h"

#define MAX_NUM_POINT_LIGHTS 256
#define MAX_NUM_SPOTLIGHTS 256

typedef struct directional_light_t
{
	kmVec3 color;
	kmVec3 ambient;
	kmQuaternion previousDirection;
	kmQuaternion direction;
} DirectionalLight;

typedef struct point_light_t
{
	kmVec3 color;
	kmVec3 ambient;
	kmVec3 previousPosition;
	kmVec3 position;
	real32 constantAttenuation;
    real32 linearAttenuation;
    real32 quadraticAttenuation;
} PointLight;

typedef struct spotlight_t
{
	kmVec3 color;
	kmVec3 ambient;
	kmVec3 previousPosition;
	kmVec3 position;
	kmQuaternion previousDirection;
	kmQuaternion direction;
	kmVec2 size;
} Spotlight;