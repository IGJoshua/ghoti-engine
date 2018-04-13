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
