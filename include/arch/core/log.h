#pragma once
#include "defines.h"

#include "core/config.h"

#include "file/utilities.h"

#include <stdio.h>

FILE *logFile;
FILE *assetLogFile;

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

#define ASSET_LOG_FILE_NAME config.logConfig.assetManagerFile
#define ASSET_LOG(...) //assetLogFile = fopen(ASSET_LOG_FILE_NAME, "a"); \
					   //fprintf(assetLogFile, __VA_ARGS__); \
					   //fclose(assetLogFile)

void logFunction(const char *format, ...);