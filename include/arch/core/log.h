#pragma once
#include "defines.h"

#include <stdio.h>

#define LOG_FILE_NAME "engine.log"

FILE *logFile;

#ifdef _DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) logFile = fopen(LOG_FILE_NAME, "a"); \
				 fprintf(logFile, __VA_ARGS__); \
				 fclose(logFile)
#endif