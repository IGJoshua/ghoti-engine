#include "core/config.h"
#include "core/log.h"

#include "file/utilities.h"

#include <cjson/cJSON.h>

#include <malloc.h>
#include <string.h>

extern Config config;

internal void initializeDefaultConfig(void);

int32 loadConfig(void)
{
	initializeDefaultConfig();

	cJSON *json = loadJSON(CONFIG_FILE);
	if (!json)
	{
		LOG("Failed to load %s.json\n", CONFIG_FILE);
		return -1;
	}

	cJSON *window = cJSON_GetObjectItem(json, "window");

	if (window)
	{
		free(config.windowConfig.title);

		cJSON *windowTitle = cJSON_GetObjectItem(window, "title");
		config.windowConfig.title = malloc(
			strlen(windowTitle->valuestring) + 1);
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
	}

	cJSON *physics = cJSON_GetObjectItem(json, "physics");

	if (physics)
	{
		cJSON *physicsFPS = cJSON_GetObjectItem(physics, "fps");
		config.physicsConfig.fps = physicsFPS->valuedouble;
	}

	cJSON *graphics = cJSON_GetObjectItem(json, "graphics");

	if (graphics)
	{
		cJSON *graphicsBackgroundColor = cJSON_GetObjectItem(
			graphics,
			"background_color");
		kmVec3Fill(
			&config.graphicsConfig.backgroundColor,
			graphicsBackgroundColor->child->valuedouble,
			graphicsBackgroundColor->child->next->valuedouble,
			graphicsBackgroundColor->child->next->next->valuedouble);
	}

	cJSON *logging = cJSON_GetObjectItem(json, "logging");

	if (logging)
	{
		free(config.logConfig.engineFile);
		free(config.logConfig.luaFile);

		cJSON *loggingEngineFile = cJSON_GetObjectItem(logging, "engine_file");
		config.logConfig.engineFile = malloc(
			strlen(loggingEngineFile->valuestring) + 1);
		strcpy(config.logConfig.engineFile, loggingEngineFile->valuestring);

		cJSON *loggingLuaFile = cJSON_GetObjectItem(logging, "lua_file");
		config.logConfig.luaFile = malloc(
			strlen(loggingLuaFile->valuestring) + 1);
		strcpy(config.logConfig.luaFile, loggingLuaFile->valuestring);
	}

	cJSON *saves = cJSON_GetObjectItem(json, "saves");

	if (saves)
	{
		cJSON *removeJSONScenes = cJSON_GetObjectItem(saves, "remove_json_scenes");
		config.savesConfig.removeJSONScenes = cJSON_IsTrue(removeJSONScenes) ?
			true : false;

		cJSON *removeJSONEntities = cJSON_GetObjectItem(
			saves,
			"remove_json_entities");
		config.savesConfig.removeJSONEntities =
			cJSON_IsTrue(removeJSONEntities) ? true : false;
	}

	cJSON *jsonConfig = cJSON_GetObjectItem(json, "json");

	if (jsonConfig)
	{
		cJSON *formatted = cJSON_GetObjectItem(jsonConfig, "formatted");
		config.jsonConfig.formatted = cJSON_IsTrue(formatted) ? true : false;
	}

	cJSON_Delete(json);

	return 0;
}

void freeConfig(void)
{
	free(config.windowConfig.title);
	free(config.logConfig.engineFile);
	free(config.logConfig.luaFile);
}

void initializeDefaultConfig(void)
{
	config.windowConfig.title = malloc(6);
	strcpy(config.windowConfig.title, "Ghoti");
	config.windowConfig.fullscreen = false;
	kmVec2Fill(&config.windowConfig.size, 640, 480);
	config.windowConfig.vsync = true;

	config.physicsConfig.fps = 60;

	kmVec3Fill(&config.graphicsConfig.backgroundColor, 0.0f, 0.0f, 0.0f);

	config.logConfig.engineFile = malloc(11);
	strcpy(config.logConfig.engineFile, "engine.log");
	config.logConfig.luaFile = malloc(8);
	strcpy(config.logConfig.luaFile, "lua.log");

	config.savesConfig.removeJSONScenes = true;
	config.savesConfig.removeJSONEntities = true;

	config.jsonConfig.formatted = true;
}