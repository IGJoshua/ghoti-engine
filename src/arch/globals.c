#include "core/config.h"

#include "asset_management/asset_manager_types.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"

#include "data/data_types.h"

#include <luajit-2.0/lua.h>

#include <pthread.h>

Config config;

// Resource HashMaps

HashMap models;
pthread_mutex_t modelsMutex;

HashMap textures;
pthread_mutex_t texturesMutex;

HashMap materialFolders;
pthread_mutex_t materialFoldersMutex;

HashMap fonts;
pthread_mutex_t fontsMutex;

HashMap images;
pthread_mutex_t imagesMutex;

HashMap audioFiles;
pthread_mutex_t audioMutex;

HashMap particles;

// Resource Loading HashMaps

HashMap loadingModels;
pthread_mutex_t loadingModelsMutex;

HashMap loadingTextures;
pthread_mutex_t loadingTexturesMutex;

HashMap loadingFonts;
pthread_mutex_t loadingFontsMutex;

HashMap loadingImages;
pthread_mutex_t loadingImagesMutex;

HashMap loadingAudio;
pthread_mutex_t loadingAudioMutex;

// Resource Uploading HashMaps

HashMap uploadModelsQueue;
pthread_mutex_t uploadModelsMutex;

HashMap uploadTexturesQueue;
pthread_mutex_t uploadTexturesMutex;

HashMap uploadFontsQueue;
pthread_mutex_t uploadFontsMutex;

HashMap uploadImagesQueue;
pthread_mutex_t uploadImagesMutex;

HashMap uploadAudioQueue;
pthread_mutex_t uploadAudioMutex;

// Asset Management Globals

uint32 assetThreadCount;
pthread_mutex_t assetThreadsMutex;
pthread_cond_t assetThreadsCondition;

pthread_mutex_t devilMutex;

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
List unloadedScenes;
bool loadingSave;
List savedScenes;

Scene *listenerScene = NULL;