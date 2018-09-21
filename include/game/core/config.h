#pragma once
#include "defines.h"

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>

#define CONFIG_FILE "ghoti"

typedef struct window_config_t
{
	char *title;
	bool fullscreen;
	kmVec2 size;
	bool vsync;
} WindowConfig;

typedef struct physics_config_t
{
	real32 fps;
} PhysicsConfig;

typedef struct graphics_config_t
{
	kmVec3 backgroundColor;
} GraphicsConfig;

typedef struct assets_config_t
{
	real64 minimumAudioLifetime;
	real64 minimumFontLifetime;
	real64 minimumImageLifetime;
	real64 minimumTextureLifetime;
	real64 minimumModelLifetime;
	real64 minimumParticleLifetime;
} AssetsConfig;

typedef struct log_config_t
{
	char *engineFile;
	char *assetManagerFile;
	char *luaFile;
} LogConfig;

typedef struct saves_config_t
{
	bool removeJSONScenes;
	bool removeJSONEntities;
} SavesConfig;

typedef struct json_config_t
{
	bool formatted;
} JSONConfig;

typedef struct config_t
{
	WindowConfig windowConfig;
	PhysicsConfig physicsConfig;
	GraphicsConfig graphicsConfig;
	AssetsConfig assetsConfig;
	LogConfig logConfig;
	SavesConfig savesConfig;
	JSONConfig jsonConfig;
} Config;

int32 loadConfig(void);
void freeConfig(void);