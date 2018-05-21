#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <GL/glew.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>

#define NUM_VERTEX_ATTRIBUTES 6

typedef struct vertex_t
{
	kmVec4 color;
	kmVec3 position;
	kmVec3 normal;
	kmVec3 tangent;
	kmVec3 bitangent;
	kmVec2 uv;
} Vertex;

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
	Mesh *meshes;
	uint32 numMeshes;
	Material *materials;
	uint32 numMaterials;
	uint32 refCount;
} Model;
