#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

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

typedef enum texture_format_e
{
	TEXTURE_FORMAT_RGBA8 = 0,
	TEXTURE_FORMAT_R8
} TextureFormat;

typedef struct texture_t
{
	UUID name;
	GLuint id;
	uint32 refCount;
} Texture;

typedef struct material_folder_t {
	UUID name;
	char *folder;
} MaterialFolder;

typedef struct model_t
{
	UUID name;
	uint32 refCount;
	UUID materialTexture;
	UUID opacityTexture;
	uint32 numSubsets;
	Subset *subsets;
} Model;

typedef struct font_t
{
	UUID name;
	struct nk_font_atlas atlas;
	struct nk_font *font;
	struct nk_draw_null_texture null;
	GLuint texture;
} Font;