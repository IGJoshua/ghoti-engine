#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <ode/ode.h>

#include <AL/alc.h>
#include <AL/al.h>

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

typedef struct audio_manager_t
{
	uint32 buffers;
	uint32 sources;
} AudioManagerComonent;

typedef struct audio_source_t
{
	uint32 id;
	real32 pitch;
	real32 gain;
} AudioSourceComponent;

typedef struct audio_file__t
{
	char name[1024];
	bool looping;
} AudioFileComponent;

typedef enum joint_type_e
{
	JOINT_TYPE_BALL_SOCKET = 0,
	JOINT_TYPE_HINGE,
	JOINT_TYPE_SLIDER,
	JOINT_TYPE_BALL_SOCKET2
} JointType;

typedef struct joint_information_component_t
{
	JointType type;
	UUID object1;
	UUID object2;
} JointInformationComponent;

typedef struct joint_list_component_t
{
	UUID jointInfo;
	UUID next;
} JointListComponent;

typedef struct joint_constraint_component_t
{
	bool loStop_bool;
	bool hiStop_bool;
	bool bounce_bool;
	bool CFM_bool;
	bool stopERP_bool;
	bool stopCFM_bool;

	real32 loStop_val;
	real32 hiStop_val;
	real32 bounce_val;
	real32 CFM_val;
	real32 stopERP_val;
	real32 stopCFM_val;

} JointConstraintComponent;

typedef struct hinge_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
	kmVec3 axis;
} HingeJointComponent;

typedef struct slider_joint_component_t
{
	dJointID id;
	kmVec3 axis;
} SliderJointComponent;

typedef struct ball_socket_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
} BallSocketJointComponent;

typedef struct ball_socket2_joint_component_t
{
	dJointID id;
	kmVec3 anchor1;
	kmVec3 anchor2;
	real32 distance;
} BallSocket2JointComponent;

typedef enum moment_of_inertia_e
{
	MOMENT_OF_INERTIA_USER = -1,
	MOMENT_OF_INERTIA_DEFAULT = 0,
	MOMENT_OF_INERTIA_SPHERE = 1,
	MOMENT_OF_INERTIA_CUBE,
	MOMENT_OF_INERTIA_CAPSULE
} MomentOfInertia;

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
	bool defaultDamping;
	real32 linearDamping;
	real32 angularDamping;
	real32 linearDampingThreshold;
	real32 angularDampingThreshold;
	real32 maxAngularSpeed;
	MomentOfInertia inertiaType;
	real32 moiParams[6];
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
	COLLISION_GEOM_TYPE_SPHERE,
	COLLISION_GEOM_TYPE_CAPSULE
} CollisionGeomType;

typedef struct collision_tree_node_t
{
	CollisionGeomType type;
	UUID collisionVolume;
	UUID nextCollider;
	bool isTrigger;
	dGeomID geomID;
} CollisionTreeNode;

typedef struct aabb_component_t
{
	kmVec3 bounds;
} BoxComponent;

typedef struct sphere_component_t
{
	real32 radius;
} SphereComponent;

typedef struct capsule_component_t
{
	real32 radius;
	real32 length;
} CapsuleComponent;

typedef struct hit_information_component_t
{
	int32 age;
	UUID volume1;
	UUID volume2;
	UUID object1;
	UUID object2;
	kmVec3 contactNormal;
	kmVec3 position;
	real32 depth;
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

typedef struct heightmap_component_t
{
	char textureName[1024];
	char heightmapName[1024];
	uint32 sizeX;
	uint32 sizeZ;
	uint32 maxHeight;
	real32 unitsPerTile;
	real32 uvScaleX;
	real32 uvScaleZ;
} HeightmapComponent;
