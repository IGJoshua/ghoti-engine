#pragma once
#include "defines.h"

#include <stdio.h>

FILE *logFile;

#ifdef _DEBUG
#define LOG_FILE_NAME NULL
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG_FILE_NAME "engine.log"
#define LOG(...) logFile = fopen(LOG_FILE_NAME, "a"); \
				 fprintf(logFile, __VA_ARGS__); \
				 fclose(logFile)
#endif