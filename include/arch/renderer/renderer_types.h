#pragma once
#include "defines.h"

#include <GL/glew.h>

#include <kazmath/vec3.h>

typedef struct mesh_t
{
	GLuint vertexBuffer;
	GLuint vertexArray;
	GLuint indexBuffer;
	uint32 numIndices;
} Mesh;

typedef enum material_component_type_e
{
	INVALID_MATERIAL_COMPONENT_TYPE = -1,
	MATERIAL_COMPONENT_TYPE_BASE = 0,
	MATERIAL_COMPONENT_TYPE_EMISSIVE,
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

typedef enum shader_type_e
{
	SHADER_VERTEX = 0,
	SHADER_GEOMETRY,
	SHADER_CONTROL,
	SHADER_EVALUATION,
	SHADER_FRAGMENT,
	SHADER_COMPUTE,
	SHADER_INVALID,
	SHADER_TYPE_COUNT
} ShaderType;

typedef struct shader_t
{
	GLuint object;
	ShaderType type;
	char *source;
} Shader;

typedef struct shader_pipeline_t
{
	GLuint object;
	uint32 shaderCount;
	Shader **shaders;
} ShaderPipeline;

typedef enum uniform_type_e
{
	UNIFORM_INVALID = -1,
	UNIFORM_MAT4 = 0,
	UNIFORM_VEC3,
	UNIFORM_BOOL,
	UNIFORM_TEXTURE_2D,
	UNIFORM_COUNT
} UniformType;

typedef struct uniform_t
{
	GLint location;
	UniformType type;
	char *name;
} Uniform;