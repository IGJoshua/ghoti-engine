#include "asset_management/asset_manager_types.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"

#include "data/data_types.h"

#include <luajit-2.0/lua.h>

// NOTE(Joshua): Asset Management globals

HashMap models;
HashMap textures;
HashMap materialFolders;
HashMap fonts;

// NOTE(Joshua): Globals for ECS

// Lua
lua_State *L;

// Physics globals
real64 alpha;

// Maps from system names as UUIDs to System structures
HashMap systemRegistry;

// List of scene pointers which will have systems run on them
List activeScenes;

bool changeScene;
bool reloadingScene;
List unloadedScenes;
bool loadingSave;
List savedScenes;
