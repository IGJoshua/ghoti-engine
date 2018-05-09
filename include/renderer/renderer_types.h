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
	kmVec3 tangent;
	kmVec3 bitangent;
	kmVec2 uv;
} Vertex;

typedef struct mesh_t
{
	GLuint colorBuffer;
	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint tangentBuffer;
	GLuint bitangentBuffer;
	GLuint uvBuffer;
	GLuint vertexArray;
	GLuint indexBuffer;
	uint32 numIndices;
} Mesh;

typedef struct mesh_data_t
{
	kmVec4 *colors;
	kmVec3 *positions;
	kmVec3 *normals;
	kmVec3 *tangents;
	kmVec3 *bitangents;
	kmVec2 *uvs;
	uint32 numVertices;
	uint32 *indices;
	uint32 numIndices;
} MeshData;

typedef enum material_type_e
{
	MATERIAL_TYPE_DEBUG,
	MATERIAL_TYPE_PBR,
	MATERIAL_TYPE_COUNT
} MaterialType;

// Material requirements:
// Material Type
// Diffuse Texture
// Specular Texture
// Normal Map
// Emissive Map
// Diffuse Value
// Specular Value
// Ambient Value
// Emissive Value
// Transparent Value
// Specular Power
// Specular Scale
// Opacity
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
	float specularPower;
	float specularScale;
	float opacity;
	uint32 subsetOffset;
} Material;

typedef enum texture_type_e
{
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_SPECULAR,
	TEXTURE_TYPE_NORMAL,
	TEXTURE_TYPE_EMISSIVE,
	TEXTURE_TYPE_COUNT
} TextureType;

typedef struct texture_t
{
	char *name;
	TextureType type;
	GLuint id;
	uint32 refCount;
} Texture;

typedef struct model_t
{
	char *name;
	Mesh mesh;
	Material *materials;
	uint32 numMaterials;
	uint32 refCount;
} Model;

typedef struct scene_t
{
	char **models;
} Scene;

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
