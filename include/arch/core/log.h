#pragma once
#include "defines.h"

#include "core/config.h"

#include "file/utilities.h"

#include <stdio.h>

FILE *logFile;

extern Config config;

#ifdef _DEBUG
#define LOG_FILE_NAME NULL
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG_FILE_NAME config.logConfig.engineFile
#define LOG(...) logFile = fopen(LOG_FILE_NAME, "a"); \
				 fprintf(logFile, __VA_ARGS__); \
				 fclose(logFile)
#endif

typedef enum asset_log_type_e
{
	ASSET_LOG_TYPE_NONE = -1,
	ASSET_LOG_TYPE_MODEL,
	ASSET_LOG_TYPE_TEXTURE,
	ASSET_LOG_TYPE_FONT,
	ASSET_LOG_TYPE_IMAGE,
	ASSET_LOG_TYPE_AUDIO,
	ASSET_LOG_TYPE_PARTICLE
} AssetLogType;

#define ASSET_LOG_FILE_NAME config.logConfig.assetManagerFile
#define ASSET_LOG(type, name, ...) assetLogWrite( \
	ASSET_LOG_TYPE_ ## type, \
	name, \
	__VA_ARGS__)

#define ASSET_LOG_FULL_TYPE(type, name, ...) assetLogWrite( \
	type, \
	name, \
	__VA_ARGS__)

#define ASSET_LOG_COMMIT(type, name) assetLogCommit( \
	ASSET_LOG_TYPE_ ## type, \
	name)

void initializeAssetLog(void);
void logFunction(const char *format, ...);
void assetLogWrite(
	AssetLogType type,
	const char *name,
	const char *format,
	...);
void assetLogCommit(AssetLogType type, const char *name);
void shutdownAssetLog(void);