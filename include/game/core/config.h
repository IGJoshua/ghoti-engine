#pragma once
#include "defines.h"

#include <kazmath/vec2.h>
#include <kazmath/vec3.h>

#define CONFIG_FILE "config"

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

typedef struct log_config_t
{
	char *engineFile;
	char *luaFile;
} LogConfig;

typedef struct config_t
{
	WindowConfig windowConfig;
	PhysicsConfig physicsConfig;
	GraphicsConfig graphicsConfig;
	LogConfig logConfig;
} Config;

int32 loadConfig(void);
void freeConfig(void);