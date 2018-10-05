#pragma once
#include "defines.h"

#include "components/component_types.h"

#include <GL/glew.h>

#include <kazmath/vec3.h>

#define NUM_VERTEX_ATTRIBUTES 9
#define NUM_BONES 4
#define MAX_BONE_COUNT 128

typedef struct vertex_t
{
	kmVec3 position;
	kmVec4 color;
	kmVec3 normal;
	kmVec3 tangent;
	kmVec3 bitangent;
	kmVec2 materialUV;
	kmVec2 maskUV;
	uint32 bones[NUM_BONES];
	real32 weights[NUM_BONES];
} Vertex;

typedef struct mesh_t
{
	GLuint vertexBuffer;
	GLuint vertexArray;
	GLuint indexBuffer;
	uint32 numVertices;
	Vertex *vertices;
	uint32 numIndices;
	uint32 *indices;
} Mesh;

typedef enum material_component_type_e
{
	INVALID_MATERIAL_COMPONENT_TYPE = -1,
	MATERIAL_COMPONENT_TYPE_AMBIENT_OCCLUSION = 0,
	MATERIAL_COMPONENT_TYPE_BASE,
	MATERIAL_COMPONENT_TYPE_EMISSIVE,
	MATERIAL_COMPONENT_TYPE_HEIGHT,
	MATERIAL_COMPONENT_TYPE_METALLIC,
	MATERIAL_COMPONENT_TYPE_NORMAL,
	MATERIAL_COMPONENT_TYPE_ROUGHNESS,
	MATERIAL_COMPONENT_TYPE_COUNT
} MaterialComponentType;

typedef struct material_component_t
{
	UUID texture;
	kmVec3 value;
} MaterialComponent;

typedef struct material_t
{
	UUID name;
	bool doubleSided;
	MaterialComponent components[MATERIAL_COMPONENT_TYPE_COUNT];
} Material;

typedef struct mask_t
{
	Material collectionMaterial;
	Material grungeMaterial;
	Material wearMaterial;
	real32 opacity;
} Mask;

typedef struct subset_t
{
	UUID name;
	Mesh mesh;
	Material material;
	Mask mask;
} Subset;

typedef struct vec3_key_frame_t
{
	real64 time;
	kmVec3 value;
} Vec3KeyFrame;

typedef struct quaternion_key_frame_t
{
	real64 time;
	kmQuaternion value;
} QuaternionKeyFrame;

typedef struct bone_t
{
	UUID name;
	uint32 numPositionKeyFrames;
	Vec3KeyFrame *positionKeyFrames;
	uint32 numRotationKeyFrames;
	QuaternionKeyFrame *rotationKeyFrames;
	uint32 numScaleKeyFrames;
	Vec3KeyFrame *scaleKeyFrames;
} Bone;

typedef struct animation_t
{
	UUID name;
	real64 duration;
	real64 fps;
	uint32 numBones;
	Bone *bones;
} Animation;

typedef struct bone_offset_t
{
	UUID name;
	TransformComponent transform;
} BoneOffset;

typedef struct skeleton_t
{
	uint32 numBoneOffsets;
	BoneOffset *boneOffsets;
} Skeleton;

typedef enum shader_type_e
{
	SHADER_INVALID = -1,
	SHADER_VERTEX = 0,
	SHADER_CONTROL,
	SHADER_EVALUATION,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT,
	SHADER_COMPUTE,
	SHADER_TYPE_COUNT
} ShaderType;

typedef struct shader_t
{
	GLuint object;
	ShaderType type;
	char *source;
} Shader;

typedef enum uniform_type_e
{
	UNIFORM_INVALID = -1,
	UNIFORM_MAT4 = 0,
	UNIFORM_VEC2,
	UNIFORM_VEC3,
	UNIFORM_BOOL,
	UNIFORM_FLOAT,
	UNIFORM_INT,
	UNIFORM_UINT,
	UNIFORM_TEXTURE_2D,
	UNIFORM_TEXTURE_CUBE_MAP,
	UNIFORM_COUNT
} UniformType;

typedef struct uniform_t
{
	GLint location;
	UniformType type;
	char *name;
} Uniform;