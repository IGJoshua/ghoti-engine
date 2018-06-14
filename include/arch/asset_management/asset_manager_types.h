#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <GL/glew.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>

#define RESOURCE_REALLOCATION_AMOUNT 16

typedef struct vertex_t
{
	kmVec3 position;
	kmVec4 color;
	kmVec3 normal;
	kmVec3 tangent;
	kmVec3 bitangent;
	kmVec2 uv[MATERIAL_COMPONENT_TYPE_COUNT];
} Vertex;

typedef struct texture_t
{
	char *name;
	GLuint id;
	uint32 refCount;
} Texture;

typedef struct model_t
{
	char *name;
	uint32 numSubsets;
	Material *materials;
	Mesh *meshes;
	uint32 refCount;
} Model;