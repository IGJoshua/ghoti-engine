#include "core/config.h"

#include "asset_management/asset_manager_types.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"

#include "data/data_types.h"

#include <luajit-2.0/lua.h>

#include <pthread.h>

Config config;

#define CREATE_ASSET(assets, Assets) \
HashMap assets; \
pthread_mutex_t assets ## Mutex; \
\
HashMap loading ## Assets; \
pthread_mutex_t loading ## Assets ## Mutex; \
\
HashMap upload ## Assets ## Queue; \
pthread_mutex_t upload ## Assets ## Mutex

CREATE_ASSET(models, Models);
CREATE_ASSET(textures, Textures);
CREATE_ASSET(fonts, Fonts);
CREATE_ASSET(images, Images);
CREATE_ASSET(audioFiles, Audio);
CREATE_ASSET(particles, Particles);
CREATE_ASSET(cubemaps, Cubemaps);

HashMap materialFolders;
pthread_mutex_t materialFoldersMutex;

uint32 assetThreadCount;
pthread_mutex_t assetThreadsMutex;
pthread_cond_t assetThreadsCondition;

uint32 totalThreadCount;
pthread_mutex_t totalThreadsMutex;
pthread_cond_t totalThreadsCondition;

bool assetManagerIsShutdown;
pthread_mutex_t assetManagerShutdownMutex;

// NOTE(Joshua): Globals for ECS

// Lua
lua_State *L;

// Physics globals
real64 alpha;

// Window globals
int32 viewportWidth;
int32 viewportHeight;

// Maps from system names as UUIDs to System structures
HashMap systemRegistry;

// List of scene pointers which will have systems run on them
List activeScenes;

bool changeScene;
bool reloadingScene;
bool reloadingAssets;
List unloadedScenes;
bool loadingSave;
List savedScenes;

Scene *listenerScene = NULL;