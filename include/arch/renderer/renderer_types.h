#pragma once
#include "defines.h"

#include <GL/glew.h>

#include <kazmath/vec4.h>

#define NUM_VERTEX_ATTRIBUTES 6

typedef struct mesh_t
{
	GLuint vertexBuffer;
	GLuint vertexArray;
	GLuint indexBuffer;
	uint32 numIndices;
} Mesh;

typedef enum material_type_e
{
	MATERIAL_TYPE_DEBUG,
	MATERIAL_TYPE_PBR,
	MATERIAL_TYPE_COUNT
} MaterialType;

typedef enum material_component_type_e
{
	INVALID_MATERIAL_COMPONENT_TYPE = -1,
	MATERIAL_COMPONENT_TYPE_DIFFUSE = 0,
	MATERIAL_COMPONENT_TYPE_SPECULAR,
	MATERIAL_COMPONENT_TYPE_NORMAL,
	MATERIAL_COMPONENT_TYPE_EMISSIVE,
	MATERIAL_COMPONENT_TYPE_AMBIENT,
	MATERIAL_COMPONENT_TYPE_COUNT
} MaterialComponentType;

typedef struct material_component_t
{
	char *texture;
	uint32 uvMap;
	kmVec4 value;
} MaterialComponent;

typedef struct material_t
{
	char *name;
	MaterialType type;
	MaterialComponent components[MATERIAL_COMPONENT_TYPE_COUNT];
} Material;

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
	UNIFORM_MAT4,
	UNIFORM_TEXTURE_2D,
	UNIFORM_COUNT
} UniformType;

typedef struct uniform_t
{
	GLuint location;
	UniformType type;
	char *name;
} Uniform;