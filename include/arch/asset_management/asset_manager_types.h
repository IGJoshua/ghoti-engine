#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <AL/al.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>

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

typedef struct audio_file_t
{
	UUID name;
	ALuint id;
	int32 channels;
	int32 sample_rate;
	int32 size;
	int16 *output;
	ALenum format;
} AudioFile;
