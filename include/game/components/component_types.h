#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <ode/ode.h>

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

typedef struct rigid_body_component_t
{
	dBodyID bodyID;
	dSpaceID spaceID;
	bool enabled;
	bool dynamic;
	bool gravity;
	real32 mass;
	kmVec3 centerOfMass;
	kmVec3 velocity;
	kmVec3 angularVel;
	real32 linearDamping;
	real32 angularDamping;
	real32 linearDampingThreshold;
	real32 angularDampingThreshold;
	real32 maxAngularSpeed;
} RigidBodyComponent;

typedef struct collision_component_t
{
	UUID collisionTree;
	UUID hitList;
	UUID lastHitList;
} CollisionComponent;

typedef enum collision_geom_type_e
{
	COLLISION_GEOM_TYPE_BOX = 0,
	COLLISION_GEOM_TYPE_SPHERE
} CollisionGeomType;

typedef struct collision_tree_node_t
{
	CollisionGeomType type;
	UUID collisionVolume;
	dGeomID geomID;
} CollisionTreeNode;

typedef struct aabb_component_t
{
	kmVec3 bounds;
} BoxComponent;

typedef struct sphere_component_t
{
	float radius;
} SphereComponent;

typedef struct hit_information_component_t
{
	int32 age;
	UUID volume1;
	UUID volume2;
	UUID object1;
	UUID object2;
	kmVec3 contactNormal;
	kmVec3 position;
	float depth;
} HitInformationComponent;

typedef struct hit_list_component_t
{
	UUID nextHit;
	UUID hit;
} HitListComponent;

typedef struct surface_information_component_t
{
	bool finiteFriction;
	real32 friction;
	real32 bounciness;
	real32 bounceVelocity;
	bool disableRolling;
	real32 rollingFriction;
	real32 spinningFriction;
} SurfaceInformationComponent;
