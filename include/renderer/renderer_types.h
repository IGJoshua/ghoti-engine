#pragma once
#include "defines.h"

#include <GL/glew.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>
#include <kazmath/mat4.h>

typedef struct vertex_t
{
	kmVec4 color;
	kmVec3 position;
	kmVec3 normal;
	kmVec2 uv;
} Vertex;

typedef struct mesh_t
{
	GLuint colorBuffer;
	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint uvBuffer;
	GLuint indexBuffer;
	GLuint vertexArray;
	uint32 numIndices;
	uint32 numVertices;
} Mesh;

enum material_type_e
{
	MATERIAL_TYPE_DEBUG,
	MATERIAL_TYPE_PBR,
	MATERIAL_TYPE_COUNT
};

// Material requirements
// Diffuse texture
// Specular texture
// Normal map
// Emissive map
// Static values for each
typedef struct material_t
{
	char *diffuseTexture;
	char *specularTexture;
	char *normalMap;
	char *emissiveMap;
	kmVec4 diffuseValue;
	kmVec4 specularValue;
	kmVec4 emissiveValue;
	real32 specularPower;
} Material;

typedef struct model_t
{
	kmMat4 transform;
	Mesh *mesh;
	Material *material;
} Model;

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
	UNIFORM_COUNT
} UniformType;

typedef struct uniform_t
{
	GLuint location;
	UniformType type;
	char *name;
} Uniform;
