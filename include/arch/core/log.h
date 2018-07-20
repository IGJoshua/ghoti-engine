#pragma once
#include "defines.h"

#include <stdio.h>

#ifdef _DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) fprintf(getLogFile(), __VA_ARGS__)
#endif

#define LOG_FILE_NAME "engine.log"

void initLog(void);
FILE* getLogFile(void);
void shutdownLog(void);