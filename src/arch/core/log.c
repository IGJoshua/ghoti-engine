#include "core/log.h"

#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include <stdarg.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>

#define MODELS_LOG_BUCKET_COUNT 521
#define TEXTURES_LOG_BUCKET_COUNT 2503
#define IMAGES_LOG_BUCKET_COUNT 521

internal HashMap modelsLog;
internal pthread_mutex_t modelsLogMutex;

internal HashMap texturesLog;
internal pthread_mutex_t texturesLogMutex;

internal HashMap imagesLog;
internal pthread_mutex_t imagesLogMutex;

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
	modelsLog = createHashMap(
		sizeof(UUID),
		sizeof(char*),
		MODELS_LOG_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&modelsLogMutex, NULL);

	texturesLog = createHashMap(
		sizeof(UUID),
		sizeof(char*),
		TEXTURES_LOG_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&texturesLogMutex, NULL);

	imagesLog = createHashMap(
		sizeof(UUID),
		sizeof(char*),
		IMAGES_LOG_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	pthread_mutex_init(&imagesLogMutex, NULL);

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
	freeHashMap(&modelsLog);
	pthread_mutex_destroy(&modelsLogMutex);

	freeHashMap(&texturesLog);
	pthread_mutex_destroy(&texturesLogMutex);

	freeHashMap(&imagesLog);
	pthread_mutex_destroy(&imagesLogMutex);

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
		case ASSET_LOG_TYPE_IMAGE:
			return imagesLog;
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
		case ASSET_LOG_TYPE_IMAGE:
			return &imagesLogMutex;
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