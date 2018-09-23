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

internal HashMap modelsLog;
internal pthread_mutex_t modelsLogMutex;

internal HashMap texturesLog;
internal pthread_mutex_t texturesLogMutex;

internal FILE *assetLogFile;

internal void lockAssetLogMutex(AssetLogType type);
internal char** getAssetLogBuffer(AssetLogType type, const char *name);
internal void addAssetLogBuffer(
	AssetLogType type,
	const char *name,
	char *logBuffer);
internal void removeAssetLogBuffer(AssetLogType type, const char *name);
internal void unlockAssetLogMutex(AssetLogType type);

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

	lockAssetLogMutex(type);

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

	unlockAssetLogMutex(type);

	free(message);
}

void assetLogCommit(AssetLogType type, const char *name)
{
	lockAssetLogMutex(type);

	char **logBuffer = getAssetLogBuffer(type, name);
	if (logBuffer && *logBuffer)
	{
		assetLogFile = fopen(ASSET_LOG_FILE_NAME, "a");
		fprintf(assetLogFile, "%s", *logBuffer);
		fclose(assetLogFile);

		free(*logBuffer);
		removeAssetLogBuffer(type, name);
	}

	unlockAssetLogMutex(type);
}

void shutdownAssetLog(void)
{
	freeHashMap(&modelsLog);
	pthread_mutex_destroy(&modelsLogMutex);

	freeHashMap(&texturesLog);
	pthread_mutex_destroy(&texturesLogMutex);
}

void lockAssetLogMutex(AssetLogType type)
{
	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			pthread_mutex_lock(&modelsLogMutex);
			break;
		case ASSET_LOG_TYPE_TEXTURE:
			pthread_mutex_lock(&texturesLogMutex);
			break;
		default:
			break;
	}
}

char** getAssetLogBuffer(AssetLogType type, const char *name)
{
	HashMap logBuffers = NULL;

	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			logBuffers = modelsLog;
			break;
		case ASSET_LOG_TYPE_TEXTURE:
			logBuffers = texturesLog;
			break;
		default:
			break;
	}

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
	HashMap logBuffers = NULL;

	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			logBuffers = modelsLog;
			break;
		case ASSET_LOG_TYPE_TEXTURE:
			logBuffers = texturesLog;
			break;
		default:
			break;
	}

	if (logBuffers)
	{
		UUID nameID = idFromName(name);
		hashMapInsert(logBuffers, &nameID, &logBuffer);
	}
}

void removeAssetLogBuffer(AssetLogType type, const char *name)
{
	HashMap logBuffers = NULL;

	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			logBuffers = modelsLog;
			break;
		case ASSET_LOG_TYPE_TEXTURE:
			logBuffers = texturesLog;
			break;
		default:
			break;
	}

	if (logBuffers)
	{
		UUID nameID = idFromName(name);
		hashMapDelete(logBuffers, &nameID);
	}
}

void unlockAssetLogMutex(AssetLogType type)
{
	switch (type)
	{
		case ASSET_LOG_TYPE_MODEL:
			pthread_mutex_unlock(&modelsLogMutex);
			break;
		case ASSET_LOG_TYPE_TEXTURE:
			pthread_mutex_unlock(&texturesLogMutex);
			break;
		default:
			break;
	}
}