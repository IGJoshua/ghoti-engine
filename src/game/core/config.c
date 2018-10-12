#include "core/config.h"
#include "core/log.h"

#include "file/utilities.h"

#include "components/light.h"

#include <cjson/cJSON.h>

#include <malloc.h>
#include <string.h>

#define GET_CONFIG_ITEM(name, key) \
cJSON *name = getConfigObject(json, key); \
if (name)

extern Config config;

internal void initializeDefaultConfig(void);
internal cJSON* getConfigObject(cJSON *json, const char *key);

internal bool cJSONToBool(cJSON *boolObject);

int32 loadConfig(void)
{
	initializeDefaultConfig();

	cJSON *json = loadJSON(CONFIG_FILE, &logFunction);
	if (!json)
	{
		LOG("Failed to load %s.json\n", CONFIG_FILE);
		return -1;
	}

	// Window Config

	GET_CONFIG_ITEM(windowTitle, "window.title")
	{
		free(config.windowConfig.title);
		config.windowConfig.title = malloc(
			strlen(windowTitle->valuestring) + 1);
		strcpy(config.windowConfig.title, windowTitle->valuestring);
	}

	GET_CONFIG_ITEM(windowIcon, "window.icon")
	{
		free(config.windowConfig.icon);
		config.windowConfig.icon = malloc(
			strlen(windowIcon->valuestring) + 1);
		strcpy(config.windowConfig.icon, windowIcon->valuestring);
	}

	GET_CONFIG_ITEM(windowFullscreen, "window.fullscreen")
	{
		config.windowConfig.fullscreen = cJSONToBool(windowFullscreen);
	}

	GET_CONFIG_ITEM(windowMaximized, "window.maximized")
	{
		config.windowConfig.maximized = cJSONToBool(windowMaximized);
	}

	GET_CONFIG_ITEM(windowSize, "window.size")
	{
		real32 width = windowSize->child->valuedouble;
		real32 height = windowSize->child->next->valuedouble;

		if (width >= 1.0f && height >= 1.0f)
		{
			kmVec2Fill(&config.windowConfig.size, width, height);
		}
	}

	GET_CONFIG_ITEM(windowResizable, "window.resizable")
	{
		config.windowConfig.resizable = cJSONToBool(windowResizable);
	}

	GET_CONFIG_ITEM(windowVSYNC, "window.vsync")
	{
		config.windowConfig.vsync = cJSONToBool(windowVSYNC);
	}

	// Physics Config

	GET_CONFIG_ITEM(physicsFPS, "physics.fps")
	{
		if (physicsFPS->valuedouble >= 5.0)
		{
			config.physicsConfig.fps = physicsFPS->valuedouble;
		}
	}

	// Graphics Config

	GET_CONFIG_ITEM(graphicsBackgroundColor, "graphics.background_color")
	{
		real32 r = graphicsBackgroundColor->child->valuedouble;
		real32 g = graphicsBackgroundColor->child->next->valuedouble;
		real32 b = graphicsBackgroundColor->child->next->next->valuedouble;

		if (r >= 0.0f && r <= 1.0f &&
			g >= 0.0f && g <= 1.0f &&
			b >= 0.0f && b <= 1.0f)
		{
			kmVec3Fill(&config.graphicsConfig.backgroundColor, r, g, b);
		}
	}

	GET_CONFIG_ITEM(pbr, "graphics.pbr.enabled")
	{
		config.graphicsConfig.pbr = cJSONToBool(pbr);
	}

	GET_CONFIG_ITEM(cubemapResolution, "graphics.pbr.ibl.cubemap.resolution")
	{
		int32 resolution = cubemapResolution->valueint;
		if (resolution > 0 && resolution % 16 == 0)
		{
			config.graphicsConfig.cubemapResolution = resolution;
			config.graphicsConfig.irradianceMapResolution = resolution / 16;
			config.graphicsConfig.prefilterMapResolution = resolution / 4;
		}
	}

	GET_CONFIG_ITEM(cubemapDebugMode, "graphics.pbr.ibl.cubemap.debug_mode")
	{
		config.graphicsConfig.cubemapDebugMode = cJSONToBool(cubemapDebugMode);
	}

	GET_CONFIG_ITEM(
		cubemapDebugMipLevel,
		"graphics.pbr.ibl.cubemap.debug_mip_level")
	{
		if (config.graphicsConfig.cubemapDebugMode)
		{
			config.graphicsConfig.cubemapDebugMipLevel =
				cubemapDebugMipLevel->valuedouble;
		}
	}

	GET_CONFIG_ITEM(shadowMapResolution, "graphics.shadows.resolution")
	{
		int32 resolution = shadowMapResolution->valueint;
		if (resolution > 0)
		{
			config.graphicsConfig.shadowMapResolution = resolution;
		}
	}

	GET_CONFIG_ITEM(
		directionalLightShadows,
		"graphics.shadows.directional_lights.enabled")
	{
		config.graphicsConfig.directionalLightShadows =
			cJSONToBool(directionalLightShadows);
	}

	GET_CONFIG_ITEM(
		directionalLightShadowBias,
		"graphics.shadows.directional_lights.bias")
	{
		config.graphicsConfig.directionalLightShadowBias[0] =
			directionalLightShadowBias->child->valuedouble;
		config.graphicsConfig.directionalLightShadowBias[1] =
			directionalLightShadowBias->child->next->valuedouble;
	}

	GET_CONFIG_ITEM(
		maxNumShadowPointLights,
		"graphics.shadows.point_lights.limit")
	{
		int32 limit = maxNumShadowPointLights->valueint;
		if (limit >= 0 &&
			limit <= config.graphicsConfig.maxNumShadowPointLights)
		{
			config.graphicsConfig.maxNumShadowPointLights = limit;
		}
	}

	GET_CONFIG_ITEM(pointLightShadowBias, "graphics.shadows.point_lights.bias")
	{
		config.graphicsConfig.pointLightShadowBias =
			pointLightShadowBias->valuedouble;
	}

	GET_CONFIG_ITEM(
		pointLightPCFDiskRadius,
		"graphics.shadows.point_lights.pcf_disk_radius")
	{
		real32 radius = pointLightPCFDiskRadius->valuedouble;
		if (radius > 0.0f)
		{
			config.graphicsConfig.pointLightPCFDiskRadius = 1.0f / radius;
		}
	}

	GET_CONFIG_ITEM(maxNumShadowSpotlights, "graphics.shadows.spotlights.limit")
	{
		int32 limit = maxNumShadowSpotlights->valueint;
		if (limit >= 0 && limit <= config.graphicsConfig.maxNumShadowSpotlights)
		{
			config.graphicsConfig.maxNumShadowSpotlights = limit;
		}
	}

	GET_CONFIG_ITEM(spotlightShadowBias, "graphics.shadows.spotlights.bias")
	{
		config.graphicsConfig.spotlightShadowBias[0] =
			spotlightShadowBias->child->valuedouble;
		config.graphicsConfig.spotlightShadowBias[1] =
			spotlightShadowBias->child->next->valuedouble;
	}

	GET_CONFIG_ITEM(grayscalePostProcess, "graphics.post_processing.grayscale")
	{
		config.graphicsConfig.grayscalePostProcess =
			cJSONToBool(grayscalePostProcess);
	}

	// Assets Config

	GET_CONFIG_ITEM(minAudioFileLifetime, "assets.minimum_lifetimes.audio")
	{
		config.assetsConfig.minAudioFileLifetime =
			minAudioFileLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minFontLifetime, "assets.minimum_lifetimes.fonts")
	{
		config.assetsConfig.minFontLifetime =
			minFontLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minImageLifetime, "assets.minimum_lifetimes.images")
	{
		config.assetsConfig.minImageLifetime =
			minImageLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minTextureLifetime, "assets.minimum_lifetimes.textures")
	{
		config.assetsConfig.minTextureLifetime =
			minTextureLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minModelLifetime, "assets.minimum_lifetimes.models")
	{
		config.assetsConfig.minModelLifetime =
			minModelLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minParticleLifetime, "assets.minimum_lifetimes.particles")
	{
		config.assetsConfig.minParticleLifetime =
			minParticleLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(minCubemapLifetime, "assets.minimum_lifetimes.cubemaps")
	{
		config.assetsConfig.minCubemapLifetime =
			minCubemapLifetime->valuedouble;
	}

	GET_CONFIG_ITEM(maxThreadCount, "assets.maximum_thread_count")
	{
		if (maxThreadCount->valueint >= 1)
		{
			config.assetsConfig.maxThreadCount = maxThreadCount->valueint;
		}
	}

	// Log Config

	GET_CONFIG_ITEM(engineFile, "log.files.engine")
	{
		free(config.logConfig.engineFile);
		config.logConfig.engineFile = malloc(
			strlen(engineFile->valuestring) + 1);
		strcpy(config.logConfig.engineFile, engineFile->valuestring);
	}

	GET_CONFIG_ITEM(assetManagerFile, "log.files.asset_manager")
	{
		free(config.logConfig.assetManagerFile);
		config.logConfig.assetManagerFile = malloc(
			strlen(assetManagerFile->valuestring) + 1);
		strcpy(
			config.logConfig.assetManagerFile,
			assetManagerFile->valuestring);
	}

	GET_CONFIG_ITEM(luaFile, "log.files.lua")
	{
		free(config.logConfig.luaFile);
		config.logConfig.luaFile = malloc(
			strlen(luaFile->valuestring) + 1);
		strcpy(config.logConfig.luaFile, luaFile->valuestring);
	}

	// Saves Config

	GET_CONFIG_ITEM(removeJSONScenes, "saves.remove_json_scenes")
	{
		config.savesConfig.removeJSONScenes = cJSONToBool(removeJSONScenes);
	}

	GET_CONFIG_ITEM(removeJSONEntities, "saves.remove_json_entities")
	{
		config.savesConfig.removeJSONEntities = cJSONToBool(removeJSONEntities);
	}

	// JSON Config

	GET_CONFIG_ITEM(formatJSONFiles, "json.formatted")
	{
		config.jsonConfig.formatted = cJSONToBool(formatJSONFiles);
	}

	cJSON_Delete(json);

	return 0;
}

void freeConfig(void)
{
	free(config.windowConfig.title);
	free(config.windowConfig.icon);
	free(config.logConfig.engineFile);
	free(config.logConfig.assetManagerFile);
	free(config.logConfig.luaFile);
}

void initializeDefaultConfig(void)
{
	config.windowConfig.title = malloc(6);
	strcpy(config.windowConfig.title, "Ghoti");
	config.windowConfig.icon = malloc(10);
	strcpy(config.windowConfig.icon, "ghoti.png");
	config.windowConfig.fullscreen = false;
	config.windowConfig.maximized = false;
	kmVec2Fill(&config.windowConfig.size, 640, 480);
	config.windowConfig.resizable = true;
	config.windowConfig.vsync = true;

	config.physicsConfig.fps = 60;

	kmVec3Fill(&config.graphicsConfig.backgroundColor, 0.0f, 0.0f, 0.0f);
	config.graphicsConfig.pbr = true;
	config.graphicsConfig.cubemapResolution = 1024;
	config.graphicsConfig.irradianceMapResolution = 64;
	config.graphicsConfig.cubemapDebugMode = false;
	config.graphicsConfig.cubemapDebugMipLevel = -1;
	config.graphicsConfig.shadowMapResolution = 4096;
	config.graphicsConfig.directionalLightShadows = true;
	config.graphicsConfig.directionalLightShadowBias[0] = 0.005f;
	config.graphicsConfig.directionalLightShadowBias[1] = 0.05f;
	config.graphicsConfig.maxNumShadowPointLights = MAX_NUM_SHADOW_POINT_LIGHTS;
	config.graphicsConfig.pointLightShadowBias = 0.15;
	config.graphicsConfig.pointLightPCFDiskRadius = 25.0f;
	config.graphicsConfig.maxNumShadowSpotlights = MAX_NUM_SHADOW_SPOTLIGHTS;
	config.graphicsConfig.spotlightShadowBias[0] = 0.005f;
	config.graphicsConfig.spotlightShadowBias[1] = 0.05f;
	config.graphicsConfig.grayscalePostProcess = false;

	config.assetsConfig.minAudioFileLifetime = 60.0;
	config.assetsConfig.minFontLifetime = 60.0;
	config.assetsConfig.minImageLifetime = 60.0;
	config.assetsConfig.minTextureLifetime = 60.0;
	config.assetsConfig.minModelLifetime = 60.0;
	config.assetsConfig.minParticleLifetime = 60.0;
	config.assetsConfig.minCubemapLifetime = 60.0;
	config.assetsConfig.maxThreadCount = 4;

	config.logConfig.engineFile = malloc(11);
	strcpy(config.logConfig.engineFile, "engine.log");
	config.logConfig.assetManagerFile = malloc(18);
	strcpy(config.logConfig.assetManagerFile, "asset_manager.log");
	config.logConfig.luaFile = malloc(8);
	strcpy(config.logConfig.luaFile, "lua.log");

	config.savesConfig.removeJSONScenes = true;
	config.savesConfig.removeJSONEntities = true;

	config.jsonConfig.formatted = true;
}

cJSON* getConfigObject(cJSON *json, const char *key)
{
	char *fullKey = malloc(strlen(key) + 1);
	strcpy(fullKey, key);

	const char *name = strtok(fullKey, ".");
	cJSON* object = cJSON_GetObjectItem(json, name);

	while (object)
	{
		name = strtok(NULL, ".");
		if (!name)
		{
			break;
		}

		object = cJSON_GetObjectItem(object, name);
	}

	free(fullKey);

	return object;
}

bool cJSONToBool(cJSON *boolObject)
{
	return cJSON_IsTrue(boolObject) ? true : false;
}