#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

#include <IL/il.h>

#include <AL/al.h>

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>
#include <kazmath/vec4.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

typedef enum texture_format_e
{
	TEXTURE_FORMAT_RGBA8 = 0,
	TEXTURE_FORMAT_R8
} TextureFormat;

typedef struct texture_t
{
	UUID name;
	uint32 refCount;
	real64 lifetime;
	GLuint id;
	ILuint devilID;
} Texture;

typedef struct material_folder_t
{
	UUID name;
	char *folder;
} MaterialFolder;

typedef struct model_t
{
	UUID name;
	uint32 refCount;
	real64 lifetime;
	UUID materialTexture;
	UUID opacityTexture;
	uint32 numSubsets;
	Subset *subsets;
	Skeleton skeleton;
	uint32 numAnimations;
	Animation *animations;
} Model;

typedef struct font_t
{
	UUID name;
	real64 lifetime;
	struct nk_font_atlas atlas;
	struct nk_font *font;
	struct nk_draw_null_texture null;
	GLuint textureID;
	const void *textureData;
	int32 textureWidth;
	int32 textureHeight;
} Font;

typedef struct image_t
{
	UUID name;
	uint32 refCount;
	real64 lifetime;
	GLuint id;
	ILuint devilID;
	GLsizei width;
	GLsizei height;
} Image;

typedef struct audio_file_t
{
	UUID name;
	real64 lifetime;
	ALuint id;
	int32 channels;
	int32 sample_rate;
	int32 size;
	int16 *data;
	ALenum format;
} AudioFile;

typedef struct particle_t
{
	UUID name;
	real64 lifetime;
	GLuint id;
	ILuint devilID;
	GLsizei width;
	GLsizei height;
	uint32 numSprites;
	GLsizei spriteWidth;
	GLsizei spriteHeight;
} Particle;