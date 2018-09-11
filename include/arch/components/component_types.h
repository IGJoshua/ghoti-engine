#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include <ode/ode.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>
#include <kazmath/quaternion.h>

// Enumerators

typedef enum camera_projection_type_e
{
	CAMERA_PROJECTION_TYPE_PERSPECTIVE = 0,
	CAMERA_PROJECTION_TYPE_ORTHOGRAPHIC,
	CAMERA_PROJECTION_TYPE_COUNT
} CameraProjectionType;

typedef enum collision_geom_type_e
{
	COLLISION_GEOM_TYPE_BOX = 0,
	COLLISION_GEOM_TYPE_SPHERE,
	COLLISION_GEOM_TYPE_CAPSULE,
	COLLISION_GEOM_TYPE_HEIGHTFIELD
} CollisionGeomType;

typedef enum debug_primitive_type_e
{
	DEBUG_PRIMITIVE_TYPE_POINT,
	DEBUG_PRIMITIVE_TYPE_LINE,
	DEBUG_PRIMITIVE_TYPE_TRANSFORM
} DebugPrimitiveType;

typedef enum joint_type_e
{
	JOINT_TYPE_BALL_SOCKET = 0,
	JOINT_TYPE_HINGE,
	JOINT_TYPE_SLIDER,
	JOINT_TYPE_BALL_SOCKET2
} JointType;

typedef enum moment_of_inertia_e
{
	MOMENT_OF_INERTIA_USER = -1,
	MOMENT_OF_INERTIA_DEFAULT = 0,
	MOMENT_OF_INERTIA_SPHERE = 1,
	MOMENT_OF_INERTIA_CUBE,
	MOMENT_OF_INERTIA_CAPSULE
} MomentOfInertia;

typedef enum pivot_e
{
	PIVOT_TOP_LEFT,
	PIVOT_TOP,
	PIVOT_TOP_RIGHT,
	PIVOT_LEFT,
	PIVOT_CENTER,
	PIVOT_RIGHT,
	PIVOT_BOTTOM_LEFT,
	PIVOT_BOTTOM,
	PIVOT_BOTTOM_RIGHT
} Pivot;

typedef enum text_alignment_e
{
	TEXT_ALIGNMENT_TOP_LEFT,
	TEXT_ALIGNMENT_TOP,
	TEXT_ALIGNMENT_TOP_RIGHT,
	TEXT_ALIGNMENT_LEFT,
	TEXT_ALIGNMENT_CENTER,
	TEXT_ALIGNMENT_RIGHT,
	TEXT_ALIGNMENT_BOTTOM_LEFT,
	TEXT_ALIGNMENT_BOTTOM,
	TEXT_ALIGNMENT_BOTTOM_RIGHT,
	TEXT_ALIGNMENT_WRAP
} TextAlignment;

// Structures

typedef struct animation_component_t
{
	UUID skeleton;
	char idleAnimation[64];
	real32 speed;
	real64 transitionDuration;
} AnimationComponent;

typedef struct animator_component_t
{
	char currentAnimation[64];
	real64 time;
	real64 duration;
	int32 loopCount;
	real32 speed;
	bool paused;
	char previousAnimation[64];
	real64 previousAnimationTime;
	real64 transitionTime;
	real64 transitionDuration;
} AnimatorComponent;

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

typedef struct obb_component_t
{
	kmVec3 bounds;
} BoxComponent;

typedef struct button_component_t
{
	char text[1024];
	bool pressed;
	bool held;
	bool released;
} ButtonComponent;

typedef struct camera_component_t
{
	real32 nearPlane;
	real32 farPlane;
	real32 aspectRatio;
	real32 fov;
	CameraProjectionType projectionType;
} CameraComponent;

typedef struct capsule_component_t
{
	real32 radius;
	real32 length;
} CapsuleComponent;

typedef struct collision_tree_node_t
{
	CollisionGeomType type;
	UUID collisionVolume;
	UUID nextCollider;
	bool isTrigger;
	dGeomID geomID;
} CollisionTreeNode;

typedef struct collision_component_t
{
	UUID collisionTree;
	UUID hitList;
	UUID lastHitList;
} CollisionComponent;

typedef struct debug_primitive_component_t
{
	bool visible;
	DebugPrimitiveType type;
	real32 size;
	kmVec3 color;
	UUID endpoint;
	kmVec3 endpointColor;
} DebugPrimitiveComponent;

typedef struct font_component_t
{
	char name[64];
	real32 size;
	kmVec4 color;
} FontComponent;

typedef struct gui_transform_component_t
{
	kmVec2 position;
	kmVec2 size;
	Pivot pivot;
} GUITransformComponent;

typedef struct heightmap_component_t
{
	dGeomID heightfieldGeom;
	char heightmapName[1024];
	char materialName[64];
	uint32 sizeX;
	uint32 sizeZ;
	uint32 maxHeight;
	real32 unitsPerTile;
	real32 uvScaleX;
	real32 uvScaleZ;
} HeightmapComponent;

typedef struct hinge_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
	kmVec3 axis;
} HingeJointComponent;

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

typedef struct image_component_t
{
	char name[64];
	kmVec4 color;
	kmVec2 position;
	kmVec2 scale;
	Pivot pivot;
} ImageComponent;

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

typedef struct joint_component_t
{
	char name[64];
} JointComponent;

typedef struct model_component_t
{
	char name[64];
	bool visible;
} ModelComponent;

typedef struct next_animation_component_t
{
	char name[64];
	int32 loopCount;
	real32 speed;
	real64 transitionDuration;
} NextAnimationComponent;

typedef struct panel_component_t
{
	bool enabled;
	kmVec4 color;
	UUID firstWidget;
} PanelComponent;

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

typedef struct slider_joint_component_t
{
	dJointID id;
	kmVec3 axis;
} SliderJointComponent;

typedef struct sphere_component_t
{
	real32 radius;
} SphereComponent;

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

typedef struct text_component_t
{
	char text[4096];
	TextAlignment alignment;
} TextComponent;

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

typedef struct widget_component_t
{
	bool enabled;
	UUID nextWidget;
} WidgetComponent;

typedef struct wireframe_component_t
{
	bool visible;
	real32 lineWidth;
	kmVec3 scale;
	bool customColor;
	kmVec3 color;
} WireframeComponent;