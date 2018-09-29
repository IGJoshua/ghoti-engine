#include "core/log.h"

#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#define CREATE_ASSET_LOG(asset) \
internal HashMap asset ## Log; \
internal pthread_mutex_t asset ## LogMutex

CREATE_ASSET_LOG(models);
CREATE_ASSET_LOG(textures);
CREATE_ASSET_LOG(fonts);
CREATE_ASSET_LOG(images);
CREATE_ASSET_LOG(audio);
CREATE_ASSET_LOG(particles);
CREATE_ASSET_LOG(cubemaps);

#define INITIALIZE_ASSET_LOG(asset, ASSET) \
asset ## Log = createHashMap( \
	sizeof(UUID), \
	sizeof(char*), \
	ASSET ## _LOG_BUCKET_COUNT, \
	(ComparisonOp)&strcmp); \
pthread_mutex_init(&asset ## LogMutex, NULL)

#define FREE_ASSET_LOG(asset) \
freeHashMap(&asset ## Log); \
pthread_mutex_destroy(&asset ## LogMutex)

#define MODELS_LOG_BUCKET_COUNT 521
#define TEXTURES_LOG_BUCKET_COUNT 2503
#define FONTS_LOG_BUCKET_COUNT 127
#define IMAGES_LOG_BUCKET_COUNT 521
#define AUDIO_LOG_BUCKET_COUNT 127
#define PARTICLES_LOG_BUCKET_COUNT 521
#define CUBEMAPS_LOG_BUCKET_COUNT 5

internal pthread_mutex_t assetLogMutex;

internal FILE *assetLogFile;

internal HashMap getAssetLog(AssetLogType type);
internal pthread_mutex_t* getAssetLogMutex(AssetLogType type);

internal char** getAssetLogBuffer(AssetLogType type, const char *name);
internal void addAssetLogBuffer(
	AssetLogType type,
	const char *name,
	char *logBuffer);
internal void removeAssetLogBuffer(AssetLogType type, const char *name);

void initializeAssetLog(void)
{
	INITIALIZE_ASSET_LOG(models, MODELS);
	INITIALIZE_ASSET_LOG(textures, TEXTURES);
	INITIALIZE_ASSET_LOG(fonts, FONTS);
	INITIALIZE_ASSET_LOG(images, IMAGES);
	INITIALIZE_ASSET_LOG(audio, AUDIO);
	INITIALIZE_ASSET_LOG(particles, PARTICLES);
	INITIALIZE_ASSET_LOG(cubemaps, CUBEMAPS);

	pthread_mutex_init(&assetLogMutex, NULL);
}

void logFunction(const char *format, ...)
{
	va_list args;
    va_start(args, format);

	char *message = malloc(strlen(format) + 4096);
	vsprintf(message, format, args);
    va_end(args);

	LOG("%s", message);

	free(message);
}

void assetLogWrite(
	AssetLogType type,
	const char *name,
	const char *format,
	...)
{
	va_list args;
    va_start(args, format);

	char *message = malloc(strlen(format) + 4096);
	vsprintf(message, format, args);
    va_end(args);

	pthread_mutex_t* logMutex = getAssetLogMutex(type);
	pthread_mutex_lock(logMutex);

	char **logBuffer = getAssetLogBuffer(type, name);
	if (!logBuffer || !*logBuffer)
	{
		char *newLogBuffer = calloc(1, strlen(message) + 1);
		strcpy(newLogBuffer, message);
		addAssetLogBuffer(type, name, newLogBuffer);
	}
	else
	{
		char *newLogBuffer = concatenateStrings(*logBuffer, NULL, message);
		free(*logBuffer);
		*logBuffer = newLogBuffer;
	}

	pthread_mutex_unlock(logMutex);

	free(message);
}

void assetLogCommit(AssetLogType type, const char *name)
{
	pthread_mutex_t* logMutex = getAssetLogMutex(type);
	pthread_mutex_lock(logMutex);

	char **logBuffer = getAssetLogBuffer(type, name);
	if (logBuffer && *logBuffer)
	{
		pthread_mutex_lock(&assetLogMutex);

		assetLogFile = fopen(ASSET_LOG_FILE_NAME, "a");
		fprintf(assetLogFile, "%s", *logBuffer);
		fclose(assetLogFile);

		pthread_mutex_unlock(&assetLogMutex);

		free(*logBuffer);
		removeAssetLogBuffer(type, name);
	}

	pthread_mutex_unlock(logMutex);
}

void shutdownAssetLog(void)
{
	FREE_ASSET_LOG(models);
	FREE_ASSET_LOG(textures);
	FREE_ASSET_LOG(fonts);
	FREE_ASSET_LOG(images);
	FREE_ASSET_LOG(audio);
	FREE_ASSET_LOG(particles);
	FREE_ASSET_LOG(cubemaps);

	pthread_mutex_destroy(&assetLogMutex);
}

HashMap getAssetLog(AssetLogType type)
{
	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			return modelsLog;
		case ASSET_LOG_TYPE_TEXTURE:
			return texturesLog;
		case ASSET_LOG_TYPE_FONT:
			return fontsLog;
		case ASSET_LOG_TYPE_IMAGE:
			return imagesLog;
		case ASSET_LOG_TYPE_AUDIO:
			return audioLog;
		case ASSET_LOG_TYPE_PARTICLE:
			return particlesLog;
		case ASSET_LOG_TYPE_CUBEMAP:
			return cubemapsLog;
		default:
			break;
	}

	return NULL;
}

pthread_mutex_t* getAssetLogMutex(AssetLogType type)
{
	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			return &modelsLogMutex;
		case ASSET_LOG_TYPE_TEXTURE:
			return &texturesLogMutex;
		case ASSET_LOG_TYPE_FONT:
			return &fontsLogMutex;
		case ASSET_LOG_TYPE_IMAGE:
			return &imagesLogMutex;
		case ASSET_LOG_TYPE_AUDIO:
			return &audioLogMutex;
		case ASSET_LOG_TYPE_PARTICLE:
			return &particlesLogMutex;
		case ASSET_LOG_TYPE_CUBEMAP:
			return &cubemapsLogMutex;
		default:
			break;
	}

	return NULL;
}

char** getAssetLogBuffer(AssetLogType type, const char *name)
{
	HashMap logBuffers = getAssetLog(type);
	if (logBuffers)
	{
		UUID nameID = idFromName(name);
		return hashMapGetData(logBuffers, &nameID);
	}

	return NULL;
}

void addAssetLogBuffer(
	AssetLogType type,
	const char *name,
	char *logBuffer)
{
	HashMap logBuffers = getAssetLog(type);
	if (logBuffers)
	{
		UUID nameID = idFromName(name);
		hashMapInsert(logBuffers, &nameID, &logBuffer);
	}
}

void removeAssetLogBuffer(AssetLogType type, const char *name)
{
	HashMap logBuffers = getAssetLog(type);
	if (logBuffers)
	{
		UUID nameID = idFromName(name);
		hashMapDelete(logBuffers, &nameID);
	}
}