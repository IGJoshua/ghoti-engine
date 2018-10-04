#pragma once
#include "defines.h"

#include "components/component_types.h"

#include <GL/glew.h>

#define MAX_NUM_POINT_LIGHTS 8
#define MAX_NUM_SPOTLIGHTS 8

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
	real32 radius;
} PointLight;

typedef struct spotlight_t
{
	kmVec3 color;
	kmVec3 ambient;
	kmVec3 previousPosition;
	kmVec3 position;
	kmQuaternion previousDirection;
	kmQuaternion direction;
	real32 radius;
	kmVec2 size;
} Spotlight;

#define MAX_NUM_SHADOW_POINT_LIGHTS 2

typedef struct shadow_point_light_t
{
	uint32 index;
	GLuint shadowMap;
	kmVec3 position;
	real32 farPlane;
} ShadowPointLight;