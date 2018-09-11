#include "core/config.h"
#include "core/log.h"

#include "file/utilities.h"

#include <cjson/cJSON.h>

#include <malloc.h>
#include <string.h>

extern Config config;

int32 loadConfig(void)
{
	cJSON *json = loadJSON(CONFIG_FILE);
	if (!json)
	{
		LOG("Failed to load %s.json\n", CONFIG_FILE);
		return -1;
	}

	cJSON *window = cJSON_GetObjectItem(json, "window");

	cJSON *windowTitle = cJSON_GetObjectItem(window, "title");
	config.windowConfig.title = malloc(strlen(windowTitle->valuestring) + 1);
	strcpy(config.windowConfig.title, windowTitle->valuestring);

	cJSON *windowFullscreen = cJSON_GetObjectItem(window, "fullscreen");
	config.windowConfig.fullscreen = cJSON_IsTrue(windowFullscreen) ?
		true : false;

	cJSON *windowSize = cJSON_GetObjectItem(window, "size");
	kmVec2Fill(
		&config.windowConfig.size,
		windowSize->child->valuedouble,
		windowSize->child->next->valuedouble);

	cJSON *windowVSYNC = cJSON_GetObjectItem(window, "vsync");
	config.windowConfig.vsync = cJSON_IsTrue(windowVSYNC) ?
		true : false;

	cJSON *physics = cJSON_GetObjectItem(json, "physics");

	cJSON *physicsFPS = cJSON_GetObjectItem(physics, "fps");
	config.physicsConfig.fps = physicsFPS->valuedouble;

	cJSON *graphics = cJSON_GetObjectItem(json, "graphics");

	cJSON *graphicsBackgroundColor = cJSON_GetObjectItem(
		graphics,
		"background_color");
	kmVec3Fill(
		&config.graphicsConfig.backgroundColor,
		graphicsBackgroundColor->child->valuedouble,
		graphicsBackgroundColor->child->next->valuedouble,
		graphicsBackgroundColor->child->next->next->valuedouble);

	cJSON *logging = cJSON_GetObjectItem(json, "logging");

	cJSON *loggingEngineFile = cJSON_GetObjectItem(logging, "engine_file");
	config.logConfig.engineFile = malloc(
		strlen(loggingEngineFile->valuestring) + 1);
	strcpy(config.logConfig.engineFile, loggingEngineFile->valuestring);

	cJSON *loggingLuaFile = cJSON_GetObjectItem(logging, "lua_file");
	config.logConfig.luaFile = malloc(
		strlen(loggingLuaFile->valuestring) + 1);
	strcpy(config.logConfig.luaFile, loggingLuaFile->valuestring);

	cJSON *saves = cJSON_GetObjectItem(json, "saves");

	cJSON *removeJSONScenes = cJSON_GetObjectItem(saves, "remove_json_scenes");
	config.savesConfig.removeJSONScenes = cJSON_IsTrue(removeJSONScenes) ?
		true : false;

	cJSON *removeJSONEntities = cJSON_GetObjectItem(
		saves,
		"remove_json_entities");
	config.savesConfig.removeJSONEntities = cJSON_IsTrue(removeJSONEntities) ?
		true : false;

	cJSON_Delete(json);

	return 0;
}

void freeConfig(void)
{
	free(config.windowConfig.title);
	free(config.logConfig.engineFile);
	free(config.logConfig.luaFile);
}