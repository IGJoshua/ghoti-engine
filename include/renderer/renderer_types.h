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

typedef enum material_type_e
{
	MATERIAL_TYPE_DEBUG,
	MATERIAL_TYPE_PBR,
	MATERIAL_TYPE_COUNT
} MaterialType;

// Material requirements:
// Material Type
// Diffuse texture
// Specular texture
// Normal map
// Emissive map
typedef struct material_t
{
	MaterialType type;
	char *diffuseTexture;
	char *specularTexture;
	char *normalMap;
	char *emissiveMap;
	kmVec3 diffuseValue;
	kmVec3 specularValue;
	kmVec3 ambientValue;
	kmVec3 emissiveValue;
	kmVec3 transparentValue;
	real32 specularPower;
	real32 specularScale;
	real32 opacity;
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
	UNIFORM_TEXTURE_2D,
	UNIFORM_COUNT
} UniformType;

typedef struct uniform_t
{
	GLuint location;
	UniformType type;
	char *name;
} Uniform;
