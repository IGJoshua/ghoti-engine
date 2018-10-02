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

typedef enum joint_type_e
{
	JOINT_TYPE_BALL_SOCKET = 0,
	JOINT_TYPE_HINGE,
	JOINT_TYPE_SLIDER,
	JOINT_TYPE_BALL_SOCKET2
} JointType;

typedef enum light_type_e
{
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT
} LightType;

typedef enum moment_of_inertia_e
{
	MOMENT_OF_INERTIA_USER = -1,
	MOMENT_OF_INERTIA_DEFAULT = 0,
	MOMENT_OF_INERTIA_SPHERE = 1,
	MOMENT_OF_INERTIA_CUBE,
	MOMENT_OF_INERTIA_CAPSULE
} MomentOfInertia;

typedef enum particle_animation_e
{
	PARTICLE_ANIMATION_FORWARD,
	PARTICLE_ANIMATION_BACKWARD,
	PARTICLE_ANIMATION_LOOP_FORWARD,
	PARTICLE_ANIMATION_LOOP_BACKWARD,
	PARTICLE_ANIMATION_BOUNCING_FORWARD,
	PARTICLE_ANIMATION_BOUNCING_BACKWARD
} ParticleAnimation;

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

typedef struct audio_source_component_t
{
	uint32 id;
	real32 pitch;
	real32 gain;
	bool looping;
} AudioSourceComponent;

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

typedef struct box_component_t
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
} CollisionTreeNodeComponent;

typedef struct collision_component_t
{
	UUID collisionTree;
	UUID hitList;
	UUID lastHitList;
} CollisionComponent;

typedef struct cubemap_component_t
{
	char name[64];
	bool swapFrontAndBack;
} CubemapComponent;

typedef struct debug_collision_primitive_component_t
{
	bool visible;
	bool recursive;
	real32 lineWidth;
	kmVec3 boxColor;
	kmVec3 sphereColor;
	kmVec3 capsuleColor;
} DebugCollisionPrimitiveComponent;

typedef struct debug_line_component_t
{
	UUID endpoint;
	real32 lineWidth;
	kmVec3 color;
	kmVec3 endpointColor;
} DebugLineComponent;

typedef struct debug_point_component_t
{
	real32 size;
	kmVec3 color;
} DebugPointComponent;

typedef struct debug_primitive_component_t
{
	bool visible;
} DebugPrimitiveComponent;

typedef struct debug_transform_component_t
{
	bool recursive;
	real32 lineWidth;
	real32 scale;
	kmVec3 xAxisColor;
	kmVec3 xAxisEndpointColor;
	kmVec3 yAxisColor;
	kmVec3 yAxisEndpointColor;
	kmVec3 zAxisColor;
	kmVec3 zAxisEndpointColor;
} DebugTransformComponent;

typedef struct font_component_t
{
	char name[64];
	real32 size;
	bool autoScaling;
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
	bool textureFiltering;
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

typedef struct light_component_t
{
	bool enabled;
	LightType type;
	kmVec3 color;
	kmVec3 ambient;
	real32 constantAttenuation;
    real32 linearAttenuation;
    real32 quadraticAttenuation;
	kmVec2 size;
} LightComponent;

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

typedef struct particle_emitter_component_t
{
	bool active;
	bool paused;
	bool stopping;
	char currentParticle[64];
	real64 particleCounter;
	real32 currentSpawnRate;
	real32 spawnRate[2];
	uint32 maxNumParticles;
	bool stopAtCapacity;
	real64 lifetime[2];
	real32 fadeTime[2];
	kmVec3 initialVelocity;
	kmVec3 minRandomVelocity;
	kmVec3 maxRandomVelocity;
	kmVec3 acceleration;
	kmVec2 minSize;
	kmVec2 maxSize;
	bool preserveAspectRatio;
	kmVec4 color;
	kmVec4 minRandomColor;
	kmVec4 maxRandomColor;
	int32 initialSprite;
	bool randomSprite;
	real32 animationFPS;
	ParticleAnimation animationMode;
	int32 finalSprite;
} ParticleEmitterComponent;

typedef struct progress_bar_component_t
{
	real32 value;
	kmVec4 color;
	kmVec4 backgroundColor;
	bool reversed;
} ProgressBarComponent;

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

typedef struct slider_component_t
{
	int32 value;
	int32 minValue;
	int32 maxValue;
	int32 stepSize;
	real32 height;
	real32 length;
	real32 buttonSize;
	kmVec4 fillColor;
	kmVec4 backgroundColor;
	kmVec4 buttonColor;
} SliderComponent;

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

typedef struct text_field_component_t
{
	char text[4096];
} TextFieldComponent;

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
	kmVec4 backgroundColor;
	UUID nextWidget;
} WidgetComponent;

typedef struct wireframe_component_t
{
	bool visible;
	real32 lineWidth;
	kmVec3 scale;
	kmVec3 color;
} WireframeComponent;