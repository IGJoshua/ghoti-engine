#pragma once
#include "defines.h"

#include "components/component_types.h"

#include <GL/glew.h>

#include <kazmath/quaternion.h>
#include <kazmath/mat4.h>

#define MAX_NUM_POINT_LIGHTS 64
#define MAX_NUM_SPOTLIGHTS 64

typedef struct directional_light_t
{
	kmVec3 radiantFlux;
	kmQuaternion previousDirection;
	kmQuaternion direction;
} DirectionalLight;

typedef struct point_light_t
{
	kmVec3 radiantFlux;
	kmVec3 previousPosition;
	kmVec3 position;
	real32 radius;
	int32 shadowIndex;
} PointLight;

typedef struct spotlight_t
{
	kmVec3 radiantFlux;
	kmVec3 previousPosition;
	kmVec3 position;
	kmQuaternion previousDirection;
	kmQuaternion direction;
	real32 radius;
	kmVec2 size;
	int32 shadowIndex;
} Spotlight;

#define MAX_NUM_SHADOW_POINT_LIGHTS 4
#define MAX_NUM_SHADOW_SPOTLIGHTS 4

typedef struct shadow_directional_light_t
{
	GLuint shadowMap;
	kmQuaternion previousDirection;
	kmQuaternion direction;
	kmMat4 transform;
} ShadowDirectionalLight;

typedef struct shadow_point_light_t
{
	GLuint shadowMap;
	kmVec3 previousPosition;
	kmVec3 position;
	real32 farPlane;
} ShadowPointLight;

typedef struct shadow_spotlight_t
{
	GLuint shadowMap;
	kmVec3 previousPosition;
	kmVec3 position;
	kmQuaternion previousDirection;
	kmQuaternion direction;
	real32 fov;
	real32 farPlane;
	kmMat4 transform;
} ShadowSpotlight;